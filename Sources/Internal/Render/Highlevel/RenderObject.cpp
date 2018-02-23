#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/ReflectionProbe.h"
#include "Render/Highlevel/Light.h"
#include "Base/ObjectFactory.h"
#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "Render/Renderer.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

ENUM_DECLARE(DAVA::RenderObject::eType)
{
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_RENDEROBJECT, "Render Object");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_MESH, "Mesh");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_SKINNED_MESH, "Skinned mesh");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_LANDSCAPE, "Landscape");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_CUSTOM_DRAW, "Custom draw");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_SPRITE, "Sprite");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_PARTICLE_EMITTER, "Particle emitter");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE__DELETED__SKYBOX, "Deleted skybox");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_VEGETATION, "Vegetation");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_SPEED_TREE, "Speed tree");
    ENUM_ADD_DESCR(DAVA::RenderObject::eType::TYPE_BILLBOARD, "Billboard");
}

ENUM_DECLARE(DAVA::RenderObject::eFlags)
{
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::VISIBLE, "Visible");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::ALWAYS_CLIPPING_VISIBLE, "Clipping always visible");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::VISIBLE_STATIC_OCCLUSION, "Visible static occlusion");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::TREE_NODE_NEED_UPDATE, "Tree node need update");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::NEED_UPDATE, "Need update");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::MARKED_FOR_UPDATE, "Marked for update");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER, "Custom prepare to render");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::VISIBLE_REFLECTION, "Visible reflection");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::VISIBLE_REFRACTION, "Visible refraction");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::VISIBLE_QUALITY, "Visible quality");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::VISIBLE_SHADOW_CASTER, "Shadow caster");
    ENUM_ADD_DESCR(DAVA::RenderObject::eFlags::TRANSFORM_UPDATED, "Transform updated");
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(RenderBatchWithOptions)
{
    ReflectionRegistrator<RenderBatchWithOptions>::Begin()
    .Field("renderBatch", &RenderBatchWithOptions::renderBatch)[M::DisplayName("Render batch")]
    .Field("lodIndex", &RenderBatchWithOptions::lodIndex)[M::DisplayName("LOD index"), M::Range(-1, 3, 1), M::ReadOnly()]
    .Field("switchIndex", &RenderBatchWithOptions::switchIndex)[M::DisplayName("Switch index"), M::Range(-1, 1, 1), M::ReadOnly()]
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RenderBatchProvider)
{
    ReflectionRegistrator<RenderBatchProvider>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RenderObject)
{
    ReflectionRegistrator<RenderObject>::Begin()
    .Field("type", &RenderObject::type)[M::DisplayName("Type"), M::EnumT<RenderObject::eType>(), M::ReadOnly()]
    .Field("flags", &RenderObject::flags)[M::DisplayName("Flags"), M::FlagsT<RenderObject::eFlags>(), M::DeveloperModeOnly()]
    .Field("debugFlags", &RenderObject::debugFlags)[M::DisplayName("Debug flags"), M::DeveloperModeOnly()]
    .Field("removeIndex", &RenderObject::removeIndex)[M::ReadOnly(), M::HiddenField()]
    .Field("bbox", &RenderObject::bbox)[M::DisplayName("Bounding box"), M::Group("Bounding boxes"), M::DeveloperModeOnly()]
    .Field("worldBBox", &RenderObject::worldBBox)[M::DisplayName("World Bounding box"), M::Group("Bounding boxes"), M::DeveloperModeOnly()]
    .Field("lodIndex", &RenderObject::GetLodIndex, &RenderObject::SetLodIndex)[M::DisplayName("LOD index"), M::Range(-1, 3, 1), M::DeveloperModeOnly()]
    // we have to put upper bound of switchIndex to 1 because in ResourceEditor we have asserts like this one DVASSERT(switch < 2)
    .Field("switchIndex", &RenderObject::GetSwitchIndex, &RenderObject::SetSwitchIndex)[M::DisplayName("Switch index"), M::Range(-1, 1, 1), M::DeveloperModeOnly()]
    .Field("visibleReflection", &RenderObject::GetReflectionVisible, &RenderObject::SetReflectionVisible)[M::DisplayName("Visible reflection")]
    .Field("visibleRefraction", &RenderObject::GetRefractionVisible, &RenderObject::SetRefractionVisible)[M::DisplayName("Visible refraction")]
    .Field("clippingVisible", &RenderObject::GetClippingVisible, &RenderObject::SetClippingVisible)[M::DisplayName("Always clipping visible")]
    .Field("renderBatchArray", &RenderObject::renderBatchArray)[M::DisplayName("Render batches")]
    .Field("activeRenderBatchArray", &RenderObject::activeRenderBatchArray)[M::DisplayName("Active render batches"), M::ReadOnly()]
    .End();
}

template <>
bool AnyCompare<RenderBatchWithOptions>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<RenderBatchWithOptions>() == v2.Get<RenderBatchWithOptions>();
}

RenderObject::RenderObject()
{
    lights[0] = nullptr;
    lights[1] = nullptr;
}

RenderObject::~RenderObject()
{
    for (RenderBatchWithOptions& batch : renderBatchArray)
        SafeRelease(batch.renderBatch);

    for (RenderBatchProvider* provider : renderBatchProviders)
        SafeRelease(provider);
}

void RenderObject::UpdateAddedRenderBatch(RenderBatch* batch)
{
    DVASSERT((batch->GetRenderObject() == nullptr) || (batch->GetRenderObject() == this));
    batch->SetRenderObject(this);

    if (renderSystem)
        renderSystem->RegisterBatch(batch);
}

void RenderObject::UpdateRemovedRenderBatch(RenderBatch* batch)
{
    batch->SetRenderObject(nullptr);
    if (renderSystem)
    {
        renderSystem->UnregisterBatch(batch);
    }
}

void RenderObject::InternalAddRenderBatchToCollection(Vector<RenderBatchWithOptions>& collection, RenderBatch* batch, int32 _lodIndex, int32 _switchIndex)
{
    batch->index = int32(collection.size());
    collection.emplace_back(SafeRetain(batch), _lodIndex, _switchIndex);
    UpdateAddedRenderBatch(batch);
    RecalcBoundingBox();

    if ((_lodIndex == lodIndex && _switchIndex == switchIndex) || (_lodIndex == -1 && _switchIndex == -1))
    {
        activeRenderBatchArray.push_back(batch);
    }
}

void RenderObject::AddRenderBatch(RenderBatch* batch, int32 _lodIndex, int32 _switchIndex)
{
    InternalAddRenderBatchToCollection(renderBatchArray, batch, _lodIndex, _switchIndex);
}

void RenderObject::InternalRemoveRenderBatchFromCollection(Vector<RenderBatchWithOptions>& collection, RenderBatch* batch)
{
    uint32 size = static_cast<uint32>(collection.size());
    for (uint32 k = 0; k < size; ++k)
    {
        if (collection[k].renderBatch == batch)
        {
            batch->index = -1;
            UpdateRemovedRenderBatch(collection[k].renderBatch);
            SafeRelease(collection[k].renderBatch);
            RemoveExchangingWithLast(collection, k);
            size--;
            k--;
        }
    }
    UpdateActiveRenderBatches();
    RecalcBoundingBox();
}

void RenderObject::RemoveRenderBatch(RenderBatch* batch)
{
    InternalRemoveRenderBatchFromCollection(renderBatchArray, batch);
}

void RenderObject::AddRenderBatch(RenderBatch* batch)
{
    AddRenderBatch(batch, -1, -1);
}

void RenderObject::RemoveRenderBatch(uint32 batchIndex)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    RenderBatch* batch = renderBatchArray[batchIndex].renderBatch;
    if (renderSystem)
        renderSystem->UnregisterBatch(batch);

    batch->SetRenderObject(nullptr);
    batch->index = -1;
    batch->Release();

    renderBatchArray[batchIndex] = renderBatchArray[size - 1];
    renderBatchArray[batchIndex].renderBatch->index = int32(batchIndex);
    renderBatchArray.pop_back();
    FindAndRemoveExchangingWithLast(activeRenderBatchArray, batch);

    RecalcBoundingBox();
}

void RenderObject::ReplaceRenderBatch(RenderBatch* oldBatch, RenderBatch* newBatch)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        if (renderBatchArray[k].renderBatch == oldBatch)
        {
            ReplaceRenderBatch(k, newBatch);
            return;
        }
    }
}

void RenderObject::ReplaceRenderBatch(uint32 batchIndex, RenderBatch* newBatch)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    RenderBatch* batch = renderBatchArray[batchIndex].renderBatch;
    renderBatchArray[batchIndex].renderBatch = newBatch;

    batch->SetRenderObject(nullptr);
    batch->index = -1;
    newBatch->SetRenderObject(this);
    newBatch->index = int32(batchIndex);

    if (renderSystem)
    {
        renderSystem->UnregisterBatch(batch);
        renderSystem->RegisterBatch(newBatch);
    }

    batch->Release();
    newBatch->Retain();

    UpdateActiveRenderBatches();
    RecalcBoundingBox();
}

void RenderObject::RemoveBatches()
{
    for (RenderBatchWithOptions& batch : renderBatchArray)
    {
        if (renderSystem != nullptr)
            renderSystem->UnregisterBatch(batch.renderBatch);

        batch.renderBatch->SetRenderObject(nullptr);
        batch.renderBatch->index = -1;
        batch.renderBatch->Release();
    }

    renderBatchArray.clear();
    activeRenderBatchArray.clear();

    RecalcBoundingBox();
}

void RenderObject::SetRenderBatchLODIndex(uint32 batchIndex, int32 newLodIndex)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    RenderBatchWithOptions& iBatch = renderBatchArray[batchIndex];
    iBatch.lodIndex = newLodIndex;

    UpdateActiveRenderBatches();
}

void RenderObject::SetRenderBatchSwitchIndex(uint32 batchIndex, int32 newSwitchIndex)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    DVASSERT(batchIndex < size);

    RenderBatchWithOptions& iBatch = renderBatchArray[batchIndex];
    iBatch.switchIndex = newSwitchIndex;

    UpdateActiveRenderBatches();
}

void RenderObject::RecalcBoundingBox()
{
    bbox.Empty();

    for (const RenderBatchWithOptions& i : renderBatchArray)
        bbox.AddAABBox(i.renderBatch->GetBoundingBox());
}

void RenderObject::CollectRenderBatches(int32 requestLodIndex, int32 requestSwitchIndex, Vector<RenderBatch*>& batches, bool includeShareLods /* = false */) const
{
    uint32 batchesCount = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < batchesCount; ++i)
    {
        const RenderBatchWithOptions& irb = renderBatchArray[i];
        if ((requestLodIndex == -1 || requestLodIndex == irb.lodIndex || (includeShareLods && irb.lodIndex == -1)) &&
            (requestSwitchIndex == -1 || requestSwitchIndex == irb.switchIndex))
        {
            batches.push_back(irb.renderBatch);
        }
    }
}

RenderObject* RenderObject::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<RenderObject>(this), "Can clone only RenderObject");
        newObject = new RenderObject();
    }

    newObject->flags = flags;
    newObject->RemoveFlag(MARKED_FOR_UPDATE);
    newObject->debugFlags = debugFlags;
    newObject->staticOcclusionIndex = staticOcclusionIndex;
    newObject->lodIndex = lodIndex;
    newObject->switchIndex = switchIndex;
    //ro->bbox = bbox;
    //ro->worldBBox = worldBBox;

    //TODO:VK: Do we need remove all renderbatches from newObject?
    DVASSERT(newObject->GetRenderBatchCount() == 0);

    uint32 size = GetRenderBatchCount();
    newObject->renderBatchArray.reserve(size);
    for (uint32 i = 0; i < size; ++i)
    {
        int32 batchLodIndex, batchSwitchIndex;
        RenderBatch* batch = GetRenderBatch(i, batchLodIndex, batchSwitchIndex)->Clone();
        newObject->AddRenderBatch(batch, batchLodIndex, batchSwitchIndex);
        batch->Release();
    }
    newObject->ownerDebugInfo = ownerDebugInfo;

    return newObject;
}

void RenderObject::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    BaseObject::SaveObject(archive);

    if (NULL != archive)
    {
        archive->SetUInt32("ro.debugflags", debugFlags);
        archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);

        //VI: save only VISIBLE flag for now. May be extended in the future
        archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);

        uint32 renderBatchCount = 0;
        KeyedArchive* batchesArch = new KeyedArchive();
        for (uint32 i = 0; i < GetRenderBatchCount(); ++i)
        {
            RenderBatch* batch = GetRenderBatch(i);
            if (batch != nullptr)
            {
                archive->SetInt32(Format("rb%d.lodIndex", i), renderBatchArray[i].lodIndex);
                archive->SetInt32(Format("rb%d.switchIndex", i), renderBatchArray[i].switchIndex);
                ScopedPtr<KeyedArchive> batchArch(new KeyedArchive());
                batch->Save(batchArch, serializationContext);
                if (batchArch->Count() > 0)
                {
                    batchArch->SetString("rb.classname", batch->GetClassName());
                }
                batchesArch->SetArchive(KeyedArchive::GenKeyFromIndex(i), batchArch);
                ++renderBatchCount;
            }
        }
        archive->SetUInt32("ro.batchCount", renderBatchCount);
        archive->SetArchive("ro.batches", batchesArch);
        batchesArch->Release();
    }
}

void RenderObject::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        debugFlags = archive->GetUInt32("ro.debugflags", 0);
        staticOcclusionIndex = static_cast<uint16>(archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX));

        //VI: load only VISIBLE flag for now. May be extended in the future.
        uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);

        flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));

        uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
        KeyedArchive* batchesArch = archive->GetArchive("ro.batches");
        for (uint32 i = 0; i < roBatchCount; ++i)
        {
            int32 batchLodIndex = archive->GetInt32(Format("rb%d.lodIndex", i), -1);
            int32 batchSwitchIndex = archive->GetInt32(Format("rb%d.switchIndex", i), -1);

            KeyedArchive* batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(i));
            if (NULL != batchArch)
            {
                RenderBatch* batch = ObjectFactory::Instance()->New<RenderBatch>(batchArch->GetString("rb.classname"));

                if (NULL != batch)
                {
                    batch->Load(batchArch, serializationContext);
                    AddRenderBatch(batch, batchLodIndex, batchSwitchIndex);
                    batch->Release();
                }
            }
        }
    }

    BaseObject::LoadObject(archive);
}

void RenderObject::SaveFlags(KeyedArchive* archive, SerializationContext* serializationContext)
{
    archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);
    archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);
}

void RenderObject::LoadFlags(KeyedArchive* archive, SerializationContext* serializationContext)
{
    staticOcclusionIndex = static_cast<uint16>(archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX));

    uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);
    flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));
}

void RenderObject::BindDynamicParameters(Camera* camera, RenderBatch* batch)
{
    DVASSERT(worldTransform != 0);

    DynamicBindings& bindings = Renderer::GetDynamicBindings();

    bindings.SetDynamicParam(DynamicBindings::PARAM_WORLD, worldTransform, reinterpret_cast<pointer_size>(worldTransform));

    if (camera && lights[0])
    {
        const Vector4& lightPositionDirection0InCameraSpace = lights[0]->CalculatePositionDirectionBindVector(camera);
        bindings.SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &lightPositionDirection0InCameraSpace, reinterpret_cast<pointer_size>(&lightPositionDirection0InCameraSpace));
        bindings.SetDynamicParam(DynamicBindings::PARAM_LIGHT0_COLOR, &lights[0]->GetColor(), reinterpret_cast<pointer_size>(lights[0]));
    }
    else
    {
        //in case we don't have light we are to bind some default values to prevent fall or using previously bound light producing strange artifacts
        bindings.SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &Vector4::Zero, reinterpret_cast<pointer_size>(&Vector4::Zero));
        bindings.SetDynamicParam(DynamicBindings::PARAM_LIGHT0_COLOR, &Color::Black, reinterpret_cast<pointer_size>(&Color::Black));
    }

    bindings.SetDynamicParam(DynamicBindings::PARAM_LOCAL_BOUNDING_BOX, &bbox, reinterpret_cast<pointer_size>(&bbox));

    if ((globalReflectionProbe != nullptr) && (globalReflectionProbe->GetCurrentTexture() != nullptr))
    {
        bindings.SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_GLOBAL_REFLECTION, globalReflectionProbe->GetCurrentTexture()->handle);
    }

    if ((localReflectionProbe != nullptr) && (localReflectionProbe->GetCurrentTexture()))
    {
        bindings.SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_LOCAL_REFLECTION, localReflectionProbe->GetCurrentTexture()->handle);
        bindings.SetDynamicParam(DynamicBindings::LOCAL_PROBE_CAPTURE_POSITION_IN_WORLDSPACE, &localReflectionProbe->GetCapturePositionInWorldSpace(), reinterpret_cast<pointer_size>(localReflectionProbe));
        bindings.SetDynamicParam(DynamicBindings::LOCAL_PROBE_CAPTURE_WORLD_TO_LOCAL_MATRIX, &localReflectionProbe->GetCaptureWorldToLocalMatrix(), reinterpret_cast<pointer_size>(localReflectionProbe));
    }

    for (const DynamicPropertyParam& param : objectDynamicProps)
        bindings.SetDynamicParam(param.propertySemantic, param.data, param.updateSemantic);

    for (const DynamicTextureParam& param : objectDynamicTextures)
        bindings.SetDynamicTexture(param.textureSemantic, param.handle);

    if (batch != nullptr && batch->index != -1)
    {
        DVASSERT(batch->renderObject == this);
        DVASSERT(batch->index < int32(renderBatchArray.size()));

        for (const DynamicPropertyParam& param : renderBatchArray[batch->index].dynamicProps)
            bindings.SetDynamicParam(param.propertySemantic, param.data, param.updateSemantic);

        for (const DynamicTextureParam& param : renderBatchArray[batch->index].dynamicTextures)
            bindings.SetDynamicTexture(param.textureSemantic, param.handle);
    }
}

void RenderObject::SetRenderSystem(RenderSystem* _renderSystem)
{
    renderSystem = _renderSystem;
}

RenderSystem* RenderObject::GetRenderSystem()
{
    return renderSystem;
}

void RenderObject::BakeGeometry(const Matrix4& transform)
{
}

void RenderObject::RecalculateWorldBoundingBox()
{
    //DVASSERT(!bbox.IsEmpty());
    //DVASSERT(worldTransform != nullptr);
    bbox.GetTransformedBox(*worldTransform, worldBBox);
}

void RenderObject::PrepareToRender(Camera* camera)
{
}

void RenderObject::SetLodIndex(int32 _lodIndex)
{
    if (lodIndex != _lodIndex)
    {
        lodIndex = _lodIndex;
        UpdateActiveRenderBatches();
    }
}

void RenderObject::SetSwitchIndex(int32 _switchIndex)
{
    if (switchIndex != _switchIndex)
    {
        switchIndex = _switchIndex;
        UpdateActiveRenderBatches();
    }
}

int32 RenderObject::GetLodIndex() const
{
    return lodIndex;
}

int32 RenderObject::GetSwitchIndex() const
{
    return switchIndex;
}

void RenderObject::UpdateActiveRenderBatchesFromCollection(const Vector<RenderBatchWithOptions>& collection)
{
    for (const RenderBatchWithOptions& irb : collection)
    {
        bool validLodIndex = (irb.lodIndex == lodIndex) || (irb.lodIndex == -1);
        bool validSwitchIndex = (irb.switchIndex == switchIndex) || (irb.switchIndex == -1);
        if (validLodIndex && validSwitchIndex)
        {
            activeRenderBatchArray.push_back(irb.renderBatch);
        }
    }
}

void RenderObject::UpdateActiveRenderBatches()
{
    activeRenderBatchArray.clear();

    UpdateActiveRenderBatchesFromCollection(renderBatchArray);

    for (RenderBatchProvider* provider : renderBatchProviders)
        UpdateActiveRenderBatchesFromCollection(provider->GetRenderBatches());
}

int32 RenderObject::GetMaxLodIndex() const
{
    int32 ret = -1;
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        const RenderBatchWithOptions& irb = renderBatchArray[i];
        ret = Max(ret, irb.lodIndex);
    }

    return ret;
}

int32 RenderObject::GetMaxLodIndexForSwitchIndex(int32 forSwitchIndex) const
{
    int32 ret = -1;
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        const RenderBatchWithOptions& irb = renderBatchArray[i];
        if (irb.switchIndex == forSwitchIndex)
        {
            ret = Max(ret, irb.lodIndex);
        }
    }

    return ret;
}

int32 RenderObject::GetMaxSwitchIndex() const
{
    int32 ret = -1;
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        const RenderBatchWithOptions& irb = renderBatchArray[i];
        ret = Max(ret, irb.switchIndex);
    }

    return ret;
}

void RenderObject::GetDataNodes(Set<DataNode*>& dataNodes)
{
    for (RenderBatchWithOptions& batch : renderBatchArray)
        batch.renderBatch->GetDataNodes(dataNodes);
}

void RenderObject::AddRenderBatchProvider(RenderBatchProvider* provider)
{
    renderBatchProviders.emplace_back(SafeRetain(provider));
    for (const RenderBatchWithOptions& batch : provider->GetRenderBatches())
    {
        UpdateAddedRenderBatch(batch.renderBatch);
    }
    UpdateActiveRenderBatches();
}

void RenderObject::SetLocalReflectionProbe(ReflectionProbe* probe)
{
    if (localReflectionProbe == probe)
        return;

    SafeRelease(localReflectionProbe);
    localReflectionProbe = SafeRetain(probe);
}

void RenderObject::SetGlobalReflectionProbe(ReflectionProbe* probe)
{
    if (globalReflectionProbe == probe)
        return;

    SafeRelease(globalReflectionProbe);
    globalReflectionProbe = SafeRetain(probe);
}

void RenderObject::RemoveRenderBatchProvider(RenderBatchProvider* provider)
{
    uint32_t index = 0;
    for (RenderBatchProvider* p : renderBatchProviders)
    {
        if (p == provider)
        {
            for (const RenderBatchWithOptions& batch : provider->GetRenderBatches())
            {
                UpdateRemovedRenderBatch(batch.renderBatch);
            }
            RemoveExchangingWithLast(renderBatchProviders, index);
            SafeRelease(p);
            break;
        }
        ++index;
    }
    UpdateActiveRenderBatches();
}

void RenderObject::SetDynamicProperty(DynamicBindings::eUniformSemantic shaderSemantic, const void* value, pointer_size updateSemantic)
{
    SetDynamicPropertyInternal(&objectDynamicProps, shaderSemantic, value, updateSemantic);
}

void RenderObject::SetDynamicProperty(RenderBatch* batch, DynamicBindings::eUniformSemantic shaderSemantic, const void* value, pointer_size updateSemantic)
{
    RenderBatchWithOptions* rb = FindRenderBatchWithOptions(batch);
    if (rb != nullptr)
        SetDynamicPropertyInternal(&rb->dynamicProps, shaderSemantic, value, updateSemantic);
}

void RenderObject::RemoveDynamicProperty(DynamicBindings::eUniformSemantic shaderSemantic)
{
    RemoveDynamicPropertyInternal(&objectDynamicProps, shaderSemantic);
}

void RenderObject::RemoveDynamicProperty(RenderBatch* batch, DynamicBindings::eUniformSemantic shaderSemantic)
{
    RenderBatchWithOptions* rb = FindRenderBatchWithOptions(batch);
    if (rb != nullptr)
        RemoveDynamicPropertyInternal(&rb->dynamicProps, shaderSemantic);
}

void RenderObject::SetDynamicTexture(DynamicBindings::eTextureSemantic textureSemantic, rhi::HTexture handle)
{
    SetDynamicTextureInternal(&objectDynamicTextures, textureSemantic, handle);
}

void RenderObject::SetDynamicTexture(RenderBatch* batch, DynamicBindings::eTextureSemantic textureSemantic, rhi::HTexture handle)
{
    RenderBatchWithOptions* rb = FindRenderBatchWithOptions(batch);
    if (rb != nullptr)
        SetDynamicTextureInternal(&rb->dynamicTextures, textureSemantic, handle);
}

void RenderObject::RemoveDynamicTexture(DynamicBindings::eTextureSemantic textureSemantic)
{
    RemoveDynamicTextureInternal(&objectDynamicTextures, textureSemantic);
}

void RenderObject::RemoveDynamicTexture(RenderBatch* batch, DynamicBindings::eTextureSemantic textureSemantic)
{
    RenderBatchWithOptions* rb = FindRenderBatchWithOptions(batch);
    if (rb != nullptr)
        RemoveDynamicTextureInternal(&rb->dynamicTextures, textureSemantic);
}

RenderBatchWithOptions* RenderObject::FindRenderBatchWithOptions(RenderBatch* batch)
{
    auto found = std::find_if(renderBatchArray.begin(), renderBatchArray.end(), [&batch](const RenderBatchWithOptions& rb) {
        return rb.renderBatch == batch;
    });

    if (found != renderBatchArray.end())
        return &(*found);
    else
        return nullptr;
}

void RenderObject::SetDynamicPropertyInternal(Vector<DynamicPropertyParam>* dynamicProps, DynamicBindings::eUniformSemantic shaderSemantic, const void* value, pointer_size updateSemantic)
{
    auto found = std::find_if(dynamicProps->begin(), dynamicProps->end(), [&shaderSemantic](const DynamicPropertyParam& params) {
        return shaderSemantic == params.propertySemantic;
    });

    if (found == dynamicProps->end())
    {
        dynamicProps->emplace_back();
        found = dynamicProps->end() - 1;
    }

    found->propertySemantic = shaderSemantic;
    found->data = value;
    found->updateSemantic = updateSemantic;
}

void RenderObject::RemoveDynamicPropertyInternal(Vector<DynamicPropertyParam>* dynamicProps, DynamicBindings::eUniformSemantic shaderSemantic)
{
    auto removed = std::remove_if(dynamicProps->begin(), dynamicProps->end(), [&shaderSemantic](const DynamicPropertyParam& params) {
        return shaderSemantic == params.propertySemantic;
    });
    dynamicProps->erase(removed, dynamicProps->end());
}

void RenderObject::SetDynamicTextureInternal(Vector<DynamicTextureParam>* dynamicTextures, DynamicBindings::eTextureSemantic textureSemantic, rhi::HTexture handle)
{
    auto found = std::find_if(dynamicTextures->begin(), dynamicTextures->end(), [&textureSemantic](const DynamicTextureParam& params) {
        return textureSemantic == params.textureSemantic;
    });

    if (found == dynamicTextures->end())
    {
        dynamicTextures->emplace_back();
        found = dynamicTextures->end() - 1;
    }

    found->textureSemantic = textureSemantic;
    found->handle = handle;
}

void RenderObject::RemoveDynamicTextureInternal(Vector<DynamicTextureParam>* dynamicTextures, DynamicBindings::eTextureSemantic textureSemantic)
{
    auto removed = std::remove_if(dynamicTextures->begin(), dynamicTextures->end(), [&textureSemantic](const DynamicTextureParam& params) {
        return textureSemantic == params.textureSemantic;
    });
    dynamicTextures->erase(removed, dynamicTextures->end());
}

bool RenderBatchWithOptions::operator==(const RenderBatchWithOptions& other) const
{
    return (renderBatch == other.renderBatch) && (lodIndex == other.lodIndex) && (switchIndex == other.switchIndex);
}
};
