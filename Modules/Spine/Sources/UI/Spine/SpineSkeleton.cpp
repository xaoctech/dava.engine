#include "UI/Spine/SpineSkeleton.h"

#include <Debug/DVAssert.h>
#include <FileSystem/File.h>
#include <Logger/Logger.h>
#include <Render/Texture.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <UI/UIControl.h>

#include <spine/spine.h>
#include <spine/extension.h>

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path)
{
    using namespace DAVA;
    FilePath imagePath = path;
    Vector<Image*> images;
    ImageSystem::Load(imagePath, images);
    DVASSERT(images.size() > 0 && images[0] != nullptr, "Failed to load image!");
    Texture* texture = Texture::CreateFromData(images[0], 0);
    images[0]->Release();
    DVASSERT(texture, "Failed to create texture!");
    self->rendererObject = texture;
    self->width = texture->GetWidth();
    self->height = texture->GetHeight();
}

void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
    using namespace DAVA;
    if (self->rendererObject)
    {
        static_cast<Texture*>(self->rendererObject)->Release();
        self->rendererObject = nullptr;
    }
}

char* _spUtil_readFile(const char* path, int* length)
{
    using namespace DAVA;
    File * fp = File::Create(path, File::READ | File::OPEN);
    DVASSERT(fp != nullptr, "Failed to read file!");
    *length = static_cast<uint32>(fp->GetSize());
    char * bytes = MALLOC(char, *length);
    fp->Read(bytes, *length);
    fp->Release();

    return bytes;
}

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

int32 maxVerticesCount(spSkeleton* mSkeleton)
{
    int32 max = 0;

    auto checkIsMax = [&max](int32 value)
    {
        if (value > max)
        {
            max = value;
        }
    };

    for (int32 i = 0, n = mSkeleton->slotsCount; i < n; i++)
    {
        spSlot* slot = mSkeleton->drawOrder[i];
        if (!slot->attachment) continue;

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
    if (mAtlas != nullptr)
    {
        spAtlas_dispose(mAtlas);
        mAtlas = nullptr;
    }

    if (mTexture)
    {
        mTexture = nullptr;
    }

    if (batchDescriptor)
    {
        batchDescriptor->vertexCount = 0;
    }
}

void SpineSkeleton::ReleaseSkeleton()
{
    if (mSkeleton != nullptr)
    {
        spSkeletonData_dispose(mSkeleton->data);
        spSkeleton_dispose(mSkeleton);
        mSkeleton = nullptr;
    }

    if (mWorldVertices != nullptr)
    {
        FREE(mWorldVertices);
        mWorldVertices = nullptr;
    }

    if (mState != nullptr)
    {
        spAnimationStateData_dispose(mState->data);
        spAnimationState_dispose(mState);
        mState = nullptr;
    }

    mAnimations.clear();
    mSkins.clear();

    for (const auto& bonePair : mBones)
    {
        bonePair.second->RemoveFromParent();
    }
    mBones.clear();

    if (batchDescriptor)
    {
        batchDescriptor->vertexCount = 0;
    }
}

void SpineSkeleton::Load(const FilePath& dataPath, const FilePath& atlasPath)
{
    if (!dataPath.Exists() || !atlasPath.Exists())
    {
        return;
    }

    if (mAtlas != nullptr)
    {
        ReleaseAtlas();
    }

    mAtlas = spAtlas_createFromFile(atlasPath.GetAbsolutePathname().c_str(), 0);
    if (mAtlas == nullptr)
    {
        //DVASSERT(false, "Error reading atlas file!");
        Logger::Error("[SpineSkeleton::Load] Error reading atlas file!");
        return;
    }
    mTexture = (Texture*)mAtlas->pages[0].rendererObject;

    if (mSkeleton != nullptr)
    {
        ReleaseSkeleton();
    }

    spSkeletonJson* json = spSkeletonJson_create(mAtlas);
    json->scale = 1.0f;
    spSkeletonData* skeletonData = spSkeletonJson_readSkeletonDataFile(json, dataPath.GetAbsolutePathname().c_str());
    if (skeletonData == nullptr)
    {
        //DVASSERT(false, json->error ? json->error : "Error reading skeleton data file!");
        Logger::Error("[SpineSkeleton::Load] %s", json->error ? json->error : "Error reading skeleton data file!");
        spSkeletonJson_dispose(json);
        return;
    }
    spSkeletonJson_dispose(json);
    mSkeleton = spSkeleton_create(skeletonData);

    mWorldVertices = MALLOC(float32, SpinePrivate::maxVerticesCount(mSkeleton));

    mState = spAnimationState_create(spAnimationStateData_create(mSkeleton->data));
    DVASSERT(mState != nullptr);
    mState->rendererObject = this;

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

    mState->listener = animationCallback;
    mNeedInitialize = true;

    mAnimations.clear();
    int32 animCount = mSkeleton->data->animationsCount;
    for (int32 i = 0; i < animCount; ++i)
    {
        spAnimation* anim = mSkeleton->data->animations[i];
        mAnimations.push_back(String(anim->name));
    }

    mSkins.clear();
    int32 skinsCount = mSkeleton->data->skinsCount;
    for (int32 i = 0; i < skinsCount; ++i)
    {
        spSkin* skin = mSkeleton->data->skins[i];
        mSkins.push_back(String(skin->name));
    }
}

void SpineSkeleton::Update(const float32 timeElapsed)
{
    if (!mSkeleton || !mState || !mAtlas) return;

    if (mAtlas->pages[0].rendererObject)
    {
        mTexture = reinterpret_cast<Texture*>(mAtlas->pages[0].rendererObject);
    }

    if (mNeedInitialize)
    {
        mNeedInitialize = false;
        spSkeleton_update(mSkeleton, 0);
        spAnimationState_update(mState, 0);
        spAnimationState_apply(mState, mSkeleton);
        spSkeleton_updateWorldTransform(mSkeleton);
    }

    spSkeleton_update(mSkeleton, timeElapsed * mTimeScale);
    spAnimationState_update(mState, timeElapsed * mTimeScale);
    spAnimationState_apply(mState, mSkeleton);
    spSkeleton_updateWorldTransform(mSkeleton);

    //for (const auto & bonePair : mBones)
    //{
    //    auto bone = spSkeleton_findBone(mSkeleton, bonePair.first.c_str());
    //    DVASSERT(bone != nullptr, "Bone was not found!");
    //    auto boneControl = bonePair.second;
    //    boneControl->SetPosition(Vector2(bone->worldX, -bone->worldY));
    //    boneControl->SetAngleInDegrees(-bone->rotation);
    //    boneControl->SetScale(Vector2(bone->scaleX, bone->scaleY));
    //    boneControl->UpdateLayout();
    //}

    // Draw
    if (!mSkeleton || !mState || !mTexture) return;

    Matrix3 transformMtx;
    //geometricData.BuildTransformMatrix(transformMtx);
    
    auto pushBatch = [&]()
    {
        mPolygon.Transform(transformMtx);

        BatchDescriptor batch;
        batch.singleColor = Color::White;
        batch.material = RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL;// background->GetMaterial();
        batch.vertexCount = static_cast<uint32>(mPolygon.GetPointCount());
        batch.indexCount = static_cast<uint32>(mSpriteClippedIndecex.size());
        batch.vertexStride = SpinePrivate::VERTICES_COMPONENTS_COUNT;
        batch.texCoordStride = SpinePrivate::TEXTURE_COMPONENTS_COUNT;
        batch.vertexPointer = (float32*)mPolygon.GetPoints();
        batch.texCoordPointer[0] = (float32*)mUVs.data();
        batch.textureSetHandle = mTexture->singleTextureSet;
        batch.samplerStateHandle = mTexture->samplerStateHandle;
        batch.indexPointer = mSpriteClippedIndecex.data();
        batch.colorStride = SpinePrivate::COLOR_STRIDE;
        batch.colorPointer = mColors.data();

        batchDescriptor->operator=(batch);
    };

    auto clearData = [&]()
    {
        mPolygon.Clear();
        mUVs.clear();
        mSpriteClippedIndecex.clear();
        mColors.clear();
    };

    for (int32 i = 0, n = mSkeleton->slotsCount; i < n; i++)
    {
        spSlot* slot = mSkeleton->drawOrder[i];
        Color color(slot->r, slot->g, slot->b, slot->a);
        if (slot->attachment)
        {
            const float32* uvs = nullptr;
            int32 verticesCount = 0;
            const uint16* triangles = nullptr;
            int32 trianglesCount = 0;
            uint16 startIndex = mPolygon.GetPointCount();

            switch (slot->attachment->type)
            {
            case SP_ATTACHMENT_REGION:
            {
                spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
                spRegionAttachment_computeWorldVertices(attachment, slot->bone, mWorldVertices);
                uvs = attachment->uvs;
                verticesCount = SpinePrivate::QUAD_VERTICES_COUNT;
                triangles = SpinePrivate::QUAD_TRIANGLES;
                trianglesCount = SpinePrivate::QUAD_TRIANGLES_COUNT;
                break;
            }
            case SP_ATTACHMENT_MESH:
            {
                spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
                spMeshAttachment_computeWorldVertices(attachment, slot, mWorldVertices);
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
            
            //color *= background->GetDrawColor();

            int32 verticesCountFinal = verticesCount / SpinePrivate::VERTICES_COMPONENTS_COUNT;
            for (int32 i = 0; i < verticesCountFinal; ++i)
            {
                mPolygon.AddPoint(Vector2(mWorldVertices[i * SpinePrivate::VERTICES_COMPONENTS_COUNT], -mWorldVertices[i * SpinePrivate::VERTICES_COMPONENTS_COUNT + 1]));
                mUVs.push_back(Vector2(uvs[i * SpinePrivate::VERTICES_COMPONENTS_COUNT], uvs[i * SpinePrivate::VERTICES_COMPONENTS_COUNT + 1]));
                mColors.push_back(color.GetRGBA());
            }
            for (int32 i = 0; i < trianglesCount; i++)
            {
                mSpriteClippedIndecex.push_back(startIndex + triangles[i]);
            }
        }

        //for (const auto & bonePair : mBones)
        //{
        //    if (slot->bone->data->name == bonePair.first)
        //    {
        //        pushBatch();
        //        clearData();
        //        bonePair.second->GetBackground()->SetColor(Color(r, g, b, a));
        //        bonePair.second->SetVisibilityFlag(false); // will be restored in SystemDraw
        //        break;
        //    }
        //}
    }

    if (mPolygon.GetPointCount() > 0)
    {
        pushBatch();
        clearData();
    }
}

void SpineSkeleton::ResetSkeleton()
{
    if (mSkeleton)
    {
        spSkeleton_setToSetupPose(mSkeleton);
    }
}

BatchDescriptor* SpineSkeleton::GetRenderBatch() const
{
    return batchDescriptor;
}

const Vector<String>& SpineSkeleton::GetAvailableAnimationsNames() const
{
    return mAnimations;
}

SpineTrackEntry* SpineSkeleton::SetAnimation(int32 trackIndex, const String& name, bool loop)
{
    if (mSkeleton != nullptr && mState != nullptr)
    {
        spAnimation* animation = spSkeletonData_findAnimation(mSkeleton->data, name.c_str());
        if (!animation)
        {
            Logger::Error("[SpineSkeleton] Animation '%s' was not found!", name.c_str());
            return nullptr;
        }
        return reinterpret_cast<SpineTrackEntry*>(spAnimationState_setAnimation(mState, trackIndex, animation, loop));
    }
    else
    {
        return nullptr;
    }
}

SpineTrackEntry* SpineSkeleton::AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay)
{
    if (mSkeleton != nullptr && mState != nullptr)
    {
        spAnimation* animation = spSkeletonData_findAnimation(mSkeleton->data, name.c_str());
        if (!animation)
        {
            Logger::Error("[SpineSkeleton] Animation '%s' was not found!", name.c_str());
            return nullptr;
        }
        return reinterpret_cast<SpineTrackEntry*>(spAnimationState_addAnimation(mState, trackIndex, animation, loop, delay));
    }
    else
    {
        return nullptr;
    }
}

SpineTrackEntry* SpineSkeleton::GetTrack(int32 trackIndex)
{
    if (mState != nullptr)
    {
        return reinterpret_cast<SpineTrackEntry*>(spAnimationState_getCurrent(mState, trackIndex));
    }
    else
    {
        return nullptr;
    }
}

void SpineSkeleton::SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration)
{
    if (mState != nullptr)
    {
        spAnimationStateData_setMixByName(mState->data, fromAnimation.c_str(), toAnimation.c_str(), duration);
    }
}

void SpineSkeleton::ClearTracks()
{
    if (mState != nullptr)
    {
        spAnimationState_clearTracks(mState);
    }
}

void SpineSkeleton::CleatTrack(int32 trackIndex)
{
    if (mState != nullptr)
    {
        spAnimationState_clearTrack(mState, trackIndex);
    }
}

void SpineSkeleton::SetTimeScale(float32 timeScale)
{
    mTimeScale = timeScale;
}

float32 SpineSkeleton::GetTimeScale() const
{
    return mTimeScale;
}

bool SpineSkeleton::SetSkin(const String& skinName)
{
    if (mSkeleton != nullptr)
    {
        int32 skin = spSkeleton_setSkinByName(mSkeleton, skinName.c_str());
        if (skin != 0)
        {
            if (mWorldVertices != nullptr)
            {
                FREE(mWorldVertices);
                mWorldVertices = nullptr;
            }
            if (mSkeleton != nullptr)
            {
                mWorldVertices = MALLOC(float32, SpinePrivate::maxVerticesCount(mSkeleton));
            }
            return true;
        }
    }
    return false;
}

const Vector<String>& SpineSkeleton::GetAvailableSkinsNames() const
{
    return mSkins;
}

SpineBone* SpineSkeleton::FindBone(const String& boneName)
{
    return nullptr;
}

}