#include "UI/Spine/Private/SpineSkeleton.h"

#include "UI/Spine/Private/SpineBone.h"
#include "UI/Spine/Private/SpineTrackEntry.h"

#include <Debug/DVAssert.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <UI/UIControl.h>

#include <spine/spine.h>

namespace DAVA
{
namespace SpinePrivate
{
static const uint16 QUAD_TRIANGLES[6] = { 0, 1, 2, 2, 3, 0 };
static const int32 QUAD_VERTICES_COUNT = 8;
static const int32 QUAD_TRIANGLES_COUNT = 6;
static const int32 VERTICES_COMPONENTS_COUNT = 2;
static const int32 TEXTURE_COMPONENTS_COUNT = 2;
static const int32 COLOR_STRIDE = 1;

int32 MaxVerticesCount(spSkeleton* skeleton)
{
    int32 max = 0;

    auto checkIsMax = [&max](int32 value)
    {
        if (value > max)
        {
            max = value;
        }
    };

    for (int32 i = 0, n = skeleton->slotsCount; i < n; i++)
    {
        spSlot* slot = skeleton->drawOrder[i];
        if (!slot->attachment)
            continue;

        switch (slot->attachment->type)
        {
        case SP_ATTACHMENT_REGION:
        {
            checkIsMax(QUAD_VERTICES_COUNT);
            break;
        }
        case SP_ATTACHMENT_MESH:
        {
            checkIsMax(((spMeshAttachment*)slot->attachment)->trianglesCount * 3);
            break;
        }
        default:
            break;
        }
    }

    return max;
}
}

SpineSkeleton::SpineSkeleton()
{
    batchDescriptor = new BatchDescriptor();
}

SpineSkeleton::~SpineSkeleton()
{
    SafeDelete(batchDescriptor);
}

void SpineSkeleton::ReleaseAtlas()
{
    if (atlas != nullptr)
    {
        spAtlas_dispose(atlas);
        atlas = nullptr;
    }

    if (currentTexture)
    {
        currentTexture = nullptr;
    }

    if (batchDescriptor)
    {
        batchDescriptor->vertexCount = 0;
    }
}

void SpineSkeleton::ReleaseSkeleton()
{
    if (skeleton != nullptr)
    {
        spSkeletonData_dispose(skeleton->data);
        spSkeleton_dispose(skeleton);
        skeleton = nullptr;
    }

    if (worldVertices != nullptr)
    {
        SafeDeleteArray(worldVertices);
        worldVertices = nullptr;
    }

    if (state != nullptr)
    {
        spAnimationStateData_dispose(state->data);
        spAnimationState_dispose(state);
        state = nullptr;
    }

    animationsNames.clear();
    skinsNames.clear();

    if (batchDescriptor)
    {
        batchDescriptor->vertexCount = 0;
    }
}

void SpineSkeleton::Load(const FilePath& dataPath, const FilePath& atlasPath)
{
    if (atlas != nullptr)
    {
        ReleaseAtlas();
    }
    if (skeleton != nullptr)
    {
        ReleaseSkeleton();
    }

    if (!dataPath.Exists() || !atlasPath.Exists())
    {
        return;
    }

    atlas = spAtlas_createFromFile(atlasPath.GetAbsolutePathname().c_str(), 0);
    if (atlas == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] Error reading atlas file!");
        return;
    }
    currentTexture = (Texture*)atlas->pages[0].rendererObject;

    spSkeletonJson* json = spSkeletonJson_create(atlas);
    json->scale = 1.0f;
    spSkeletonData* skeletonData = spSkeletonJson_readSkeletonDataFile(json, dataPath.GetAbsolutePathname().c_str());
    if (skeletonData == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] %s", json->error ? json->error : "Error reading skeleton data file!");
        spSkeletonJson_dispose(json);
        if (atlas != nullptr)
        {
            ReleaseAtlas();
        }
        return;
    }
    spSkeletonJson_dispose(json);
    skeleton = spSkeleton_create(skeletonData);

    worldVertices = new float32[SpinePrivate::MaxVerticesCount(skeleton)];

    state = spAnimationState_create(spAnimationStateData_create(skeleton->data));
    DVASSERT(state != nullptr);
    if (state == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] %s", "Error creating animation state!");
        if (atlas != nullptr)
        {
            ReleaseAtlas();
        }
        if (skeleton != nullptr)
        {
            ReleaseSkeleton();
        }
        return;
    }
    state->rendererObject = this;

    auto animationCallback = [](spAnimationState* state, int32 trackIndex, spEventType type, spEvent* event, int32 loopCount)
    {
        switch (type)
        {
        case SP_ANIMATION_START:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onStart.Emit(trackIndex);
            break;
        case SP_ANIMATION_END:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onFinish.Emit(trackIndex);
            break;
        case SP_ANIMATION_COMPLETE:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onComplete.Emit(trackIndex);
            break;
        case SP_ANIMATION_EVENT:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onEvent.Emit(trackIndex, (event && event->stringValue) ? String(event->stringValue) : "");
            break;
        }
    };

    state->listener = animationCallback;

    animationsNames.clear();
    int32 animCount = skeleton->data->animationsCount;
    for (int32 i = 0; i < animCount; ++i)
    {
        spAnimation* anim = skeleton->data->animations[i];
        animationsNames.push_back(String(anim->name));
    }

    skinsNames.clear();
    int32 skinsCount = skeleton->data->skinsCount;
    for (int32 i = 0; i < skinsCount; ++i)
    {
        spSkin* skin = skeleton->data->skins[i];
        skinsNames.push_back(String(skin->name));
    }

    // Init initial state
    spSkeleton_update(skeleton, 0);
    spAnimationState_update(state, 0);
    spAnimationState_apply(state, skeleton);
    spSkeleton_updateWorldTransform(skeleton);
}

void SpineSkeleton::Update(const float32 timeElapsed)
{
    if (!skeleton || !state)
        return;

    spSkeleton_update(skeleton, timeElapsed * timeScale);
    spAnimationState_update(state, timeElapsed * timeScale);
    spAnimationState_apply(state, skeleton);
    spSkeleton_updateWorldTransform(skeleton);

    // Draw
    if (!skeleton || !state || !currentTexture)
        return;

    Matrix3 transformMtx;
    //geometricData.BuildTransformMatrix(transformMtx);

    auto pushBatch = [&]()
    {
        verticesPolygon.Transform(transformMtx);

        BatchDescriptor batch;
        batch.singleColor = Color::White;
        batch.material = RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL; // background->GetMaterial();
        batch.vertexCount = static_cast<uint32>(verticesPolygon.GetPointCount());
        batch.indexCount = static_cast<uint32>(clippedIndecex.size());
        batch.vertexStride = SpinePrivate::VERTICES_COMPONENTS_COUNT;
        batch.texCoordStride = SpinePrivate::TEXTURE_COMPONENTS_COUNT;
        batch.vertexPointer = (float32*)verticesPolygon.GetPoints();
        batch.texCoordPointer[0] = (float32*)verticesUVs.data();
        batch.textureSetHandle = currentTexture->singleTextureSet;
        batch.samplerStateHandle = currentTexture->samplerStateHandle;
        batch.indexPointer = clippedIndecex.data();
        batch.colorStride = SpinePrivate::COLOR_STRIDE;
        batch.colorPointer = verticesColors.data();

        batchDescriptor->operator=(batch);
    };

    auto clearData = [&]()
    {
        verticesPolygon.Clear();
        verticesUVs.clear();
        clippedIndecex.clear();
        verticesColors.clear();
    };

    auto switchTexture = [&](Texture* texture)
    {
        if (texture != currentTexture)
        {
            pushBatch();
            clearData();
            currentTexture = texture;
        }
    };

    for (int32 i = 0, n = skeleton->slotsCount; i < n; i++)
    {
        spSlot* slot = skeleton->drawOrder[i];
        Color color(slot->r, slot->g, slot->b, slot->a);
        if (slot->attachment)
        {
            const float32* uvs = nullptr;
            int32 verticesCount = 0;
            const uint16* triangles = nullptr;
            int32 trianglesCount = 0;
            uint16 startIndex = verticesPolygon.GetPointCount();

            switch (slot->attachment->type)
            {
            case SP_ATTACHMENT_REGION:
            {
                spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
                spRegionAttachment_computeWorldVertices(attachment, slot->bone, worldVertices);

                switchTexture(reinterpret_cast<Texture*>(reinterpret_cast<spAtlasRegion*>(attachment->rendererObject)->page->rendererObject));

                uvs = attachment->uvs;
                verticesCount = SpinePrivate::QUAD_VERTICES_COUNT;
                triangles = SpinePrivate::QUAD_TRIANGLES;
                trianglesCount = SpinePrivate::QUAD_TRIANGLES_COUNT;
                break;
            }
            case SP_ATTACHMENT_MESH:
            {
                spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
                spMeshAttachment_computeWorldVertices(attachment, slot, worldVertices);

                switchTexture(reinterpret_cast<Texture*>(reinterpret_cast<spAtlasRegion*>(attachment->rendererObject)->page->rendererObject));

                uvs = attachment->uvs;
                verticesCount = attachment->trianglesCount * 3;
                triangles = attachment->triangles;
                trianglesCount = attachment->trianglesCount;
                break;
            }
            default:
                DVASSERT(false, "Error: Wrong spine-attachment type!");
                break;
            }

            // TODO: mix colors from control
            // TODO: store slot color for bone

            int32 verticesCountFinal = verticesCount / SpinePrivate::VERTICES_COMPONENTS_COUNT;
            for (int32 i = 0; i < verticesCountFinal; ++i)
            {
                Vector2 point(worldVertices[i * SpinePrivate::VERTICES_COMPONENTS_COUNT], -worldVertices[i * SpinePrivate::VERTICES_COMPONENTS_COUNT + 1]);
                Vector2 uv(uvs[i * SpinePrivate::VERTICES_COMPONENTS_COUNT], uvs[i * SpinePrivate::VERTICES_COMPONENTS_COUNT + 1]);

                verticesPolygon.AddPoint(point);
                verticesUVs.push_back(uv);
                verticesColors.push_back(color.GetRGBA());
            }
            for (int32 i = 0; i < trianglesCount; i++)
            {
                clippedIndecex.push_back(startIndex + triangles[i]);
            }
        }
    }

    if (verticesPolygon.GetPointCount() > 0)
    {
        pushBatch();
        clearData();
    }
}

void SpineSkeleton::ResetSkeleton()
{
    if (skeleton)
    {
        spSkeleton_setToSetupPose(skeleton);
    }
}

BatchDescriptor* SpineSkeleton::GetRenderBatch() const
{
    return batchDescriptor;
}

const Vector<String>& SpineSkeleton::GetAvailableAnimationsNames() const
{
    return animationsNames;
}

std::shared_ptr<SpineTrackEntry> SpineSkeleton::SetAnimation(int32 trackIndex, const String& name, bool loop)
{
    if (skeleton != nullptr && state != nullptr)
    {
        spAnimation* animation = spSkeletonData_findAnimation(skeleton->data, name.c_str());
        if (!animation)
        {
            Logger::Error("[SpineSkeleton] Animation '%s' was not found!", name.c_str());
            return nullptr;
        }
        return std::make_shared<SpineTrackEntry>(spAnimationState_setAnimation(state, trackIndex, animation, loop));
    }
    return nullptr;
}

std::shared_ptr<SpineTrackEntry> SpineSkeleton::AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay)
{
    if (skeleton != nullptr && state != nullptr)
    {
        spAnimation* animation = spSkeletonData_findAnimation(skeleton->data, name.c_str());
        if (!animation)
        {
            Logger::Error("[SpineSkeleton] Animation '%s' was not found!", name.c_str());
            return nullptr;
        }
        return std::make_shared<SpineTrackEntry>(spAnimationState_addAnimation(state, trackIndex, animation, loop, delay));
    }
    return nullptr;
}

std::shared_ptr<SpineTrackEntry> SpineSkeleton::GetTrack(int32 trackIndex)
{
    if (state != nullptr)
    {
        return std::make_shared<SpineTrackEntry>(spAnimationState_getCurrent(state, trackIndex));
    }
    return nullptr;
}

void SpineSkeleton::SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration)
{
    if (state != nullptr)
    {
        spAnimationStateData_setMixByName(state->data, fromAnimation.c_str(), toAnimation.c_str(), duration);
    }
}

void SpineSkeleton::ClearTracks()
{
    if (state != nullptr)
    {
        spAnimationState_clearTracks(state);
    }
}

void SpineSkeleton::CleatTrack(int32 trackIndex)
{
    if (state != nullptr)
    {
        spAnimationState_clearTrack(state, trackIndex);
    }
}

void SpineSkeleton::SetTimeScale(float32 timeScale_)
{
    timeScale = timeScale_;
}

float32 SpineSkeleton::GetTimeScale() const
{
    return timeScale;
}

bool SpineSkeleton::SetSkin(const String& skinName)
{
    if (skeleton != nullptr)
    {
        int32 skin = spSkeleton_setSkinByName(skeleton, skinName.c_str());
        if (skin != 0)
        {
            SafeDeleteArray(worldVertices);
            if (skeleton != nullptr)
            {
                worldVertices = new float32[SpinePrivate::MaxVerticesCount(skeleton)];
            }
            return true;
        }
    }
    return false;
}

String SpineSkeleton::GetSkinName() const
{
    if (skeleton != nullptr && skeleton->skin != nullptr)
    {
        return String(skeleton->skin->name);
    }
    return String();
}

const Vector<String>& SpineSkeleton::GetAvailableSkinsNames() const
{
    return skinsNames;
}

std::shared_ptr<SpineBone> SpineSkeleton::FindBone(const String& boneName)
{
    if (skeleton)
    {
        return std::make_shared<SpineBone>(spSkeleton_findBone(skeleton, boneName.c_str()));
    }
    return nullptr;
}
}