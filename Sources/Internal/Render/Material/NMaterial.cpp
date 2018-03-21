#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialManager.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "Reflection/ReflectionRegistrator.h"

#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Texture.h"

#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"

#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#include "Debug/Backtrace.h"

#include "Logger/Logger.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#define TRACE_MATERIAL_REBUILDS 0

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NMaterial)
{
    ReflectionRegistrator<NMaterial>::Begin()
    .End();
}

namespace NMaterialDetail
{
template <typename K, typename V>
V GetValuePtr(const UnorderedMap<K, V>& umap, const K& key)
{
    static_assert(std::is_pointer<V>::value, "Value must be a pointer.");

    auto it = umap.find(key);

    if (it != umap.end())
    {
        return it->second;
    }

    return nullptr;
}
}

struct MaterialPropertyBinding
{
    rhi::ShaderProp::Type type;
    uint32 reg;
    uint32 regCount;
    uint32 updateSemantic;
    NMaterialProperty* source;
    MaterialPropertyBinding(rhi::ShaderProp::Type type_, uint32 reg_, uint32 regCount_, uint32 updateSemantic_, NMaterialProperty* source_)
        : type(type_)
        , reg(reg_)
        , regCount(regCount_)
        , updateSemantic(updateSemantic_)
        , source(source_)
    {
    }
};

struct MaterialBufferBinding
{
    rhi::HConstBuffer constBuffer;
    Vector<MaterialPropertyBinding> propBindings;
    uint32 lastValidPropertySemantic = 0;
};

uint32 NMaterialProperty::globalPropertyUpdateSemanticCounter = 0;

RenderVariantInstance::~RenderVariantInstance()
{
    rhi::ReleaseTextureSet(textureSet);
    rhi::ReleaseSamplerState(samplerState);
}

MaterialConfig::MaterialConfig()
    : localProperties(16)
    , localTextures(8)
    , localFlags(16)
{
}

MaterialConfig::MaterialConfig(const MaterialConfig& config)
    : localProperties(16)
    , localTextures(8)
    , localFlags(16)
{
    operator=(config);
}

MaterialConfig& MaterialConfig::operator=(const MaterialConfig& config)
{
    Clear();
    name = config.name;
    fxName = config.fxName;
    localFlags = config.localFlags;
    for (auto& tex : config.localTextures)
    {
        MaterialTextureInfo* texInfo = new MaterialTextureInfo();
        texInfo->path = tex.second->path;
        texInfo->texture = tex.second->texture;
        localTextures[tex.first] = texInfo;
    }

    for (auto& prop : config.localProperties)
    {
        NMaterialProperty* newProp = new NMaterialProperty();
        newProp->name = prop.first;
        newProp->type = prop.second->type;
        newProp->arraySize = prop.second->arraySize;
        newProp->data.reset(new float[ShaderDescriptor::CalculateDataSize(newProp->type, newProp->arraySize)]);
        newProp->SetPropertyValue(prop.second->data.get());
        localProperties[newProp->name] = newProp;
    }
    return *this;
}

void MaterialConfig::Clear()
{
    for (auto& prop : localProperties)
        SafeDelete(prop.second);

    localProperties.clear();
    for (auto& texInfo : localTextures)
    {
        SafeDelete(texInfo.second);
    }
    localTextures.clear();
}

MaterialConfig::~MaterialConfig()
{
    Clear();
}

NMaterial::NMaterial(FXDescriptor::eType type)
    : materialConfigs(1) //at least one config to emulate regular work
    , materialType(type)
    , renderVariants(4)
{
    NMaterialManager::Instance().RegisterMaterial(this);
    materialConfigs[0].name = NMaterialSerializationKey::DefaultConfigName;
}

NMaterial::~NMaterial()
{
    GetEngineContext()->assetManager->UnregisterListener(this);
    NMaterialManager::Instance().UnregisterMaterial(this);
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SetParent(nullptr);
    DVASSERT(children.empty()); //as children reference parent in our material scheme, this should not be released while it has children

    for (auto& buffer : localConstBuffers)
    {
        rhi::DeleteConstBuffer(buffer.second->constBuffer);
        SafeDelete(buffer.second);
    }
    for (auto& variant : renderVariants)
        delete variant.second;
}

void NMaterial::BindParams(rhi::Packet& target)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    //Logger::Info( "bind-params" );
    DVASSERT(activeVariantInstance); //trying to bind material that was not staged to render
    DVASSERT(activeVariantInstance->shader); //should have returned false on PreBuild!
    DVASSERT(activeVariantInstance->shader->IsValid()); //should have returned false on PreBuild!
    /*set pipeline state*/
    target.renderPipelineState = activeVariantInstance->shader->GetPiplineState();
    target.depthStencilState = activeVariantInstance->depthState;
    target.samplerState = activeVariantInstance->samplerState;
    target.textureSet = activeVariantInstance->textureSet;
    target.cullMode = activeVariantInstance->cullMode;

    if (activeVariantInstance->wireFrame)
        target.options |= rhi::Packet::OPT_WIREFRAME;
    else
        target.options &= ~rhi::Packet::OPT_WIREFRAME;

    if (activeVariantInstance->alphablend)
        target.userFlags |= USER_FLAG_ALPHABLEND;
    else
        target.userFlags &= ~USER_FLAG_ALPHABLEND;

    if (activeVariantInstance->alphatest)
        target.userFlags |= USER_FLAG_ALPHATEST;
    else
        target.userFlags &= ~USER_FLAG_ALPHATEST;

    activeVariantInstance->shader->UpdateDynamicParams();
    /*update values in material const buffers*/
    for (auto& materialBufferBinding : activeVariantInstance->materialBufferBindings)
    {
        if (materialBufferBinding->lastValidPropertySemantic == NMaterialProperty::GetCurrentUpdateSemantic()) //prevent buffer update if nothing changed
            continue;
        //assume that if we have no property - we bind default value on buffer allocation step - no binding is created in that case
        for (auto& materialBinding : materialBufferBinding->propBindings)
        {
            DVASSERT(materialBinding.source);
            if (materialBinding.updateSemantic != materialBinding.source->updateSemantic)
            {
                //Logger::Info( " upd-prop " );
                if (materialBinding.type < rhi::ShaderProp::TYPE_FLOAT4)
                {
                    DVASSERT(materialBinding.source->arraySize == 1);
                    rhi::UpdateConstBuffer1fv(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.regCount, materialBinding.source->data.get(), ShaderDescriptor::CalculateDataSize(materialBinding.type, materialBinding.source->arraySize));
                }
                else
                {
                    DVASSERT(materialBinding.source->arraySize <= materialBinding.regCount);
                    rhi::UpdateConstBuffer4fv(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.source->data.get(), ShaderDescriptor::CalculateRegsCount(materialBinding.type, materialBinding.source->arraySize));
                }
                materialBinding.updateSemantic = materialBinding.source->updateSemantic;

#if defined(__DAVAENGINE_RENDERSTATS__)
                ++Renderer::GetRenderStats().materialParamBindCount;
#endif
            }
        }
        materialBufferBinding->lastValidPropertySemantic = NMaterialProperty::GetCurrentUpdateSemantic();
    }

    target.vertexConstCount = static_cast<uint32>(activeVariantInstance->vertexConstBuffers.size());
    target.fragmentConstCount = static_cast<uint32>(activeVariantInstance->fragmentConstBuffers.size());
    /*bind material const buffers*/
    for (size_t i = 0, sz = activeVariantInstance->vertexConstBuffers.size(); i < sz; ++i)
        target.vertexConst[i] = activeVariantInstance->vertexConstBuffers[i];
    for (size_t i = 0, sz = activeVariantInstance->fragmentConstBuffers.size(); i < sz; ++i)
        target.fragmentConst[i] = activeVariantInstance->fragmentConstBuffers[i];

    for (uint32 i = 0, size = activeVariantInstance->dynamicTextureCount; i < size; ++i)
    {
        auto bindInfo = activeVariantInstance->dynamicTextureBindInfo[i];
        rhi::HTexture textureHandle = Renderer::GetDynamicBindings().GetDynamicTexture(bindInfo.semantic);

        if (!textureHandle.IsValid())
            textureHandle = Renderer::GetRuntimeTextures().GetPinkTexture(bindInfo.type);

        rhi::UpdateDynamicTexture(target.textureSet, bindInfo.isVertex, bindInfo.index, textureHandle);
    }
}

uint32 NMaterial::GetRequiredVertexFormat()
{
    uint32 res = 0;
    for (auto& variant : renderVariants)
    {
        bool shaderValid = (variant.second != nullptr) && (variant.second->shader != nullptr) && (variant.second->shader->IsValid());
        // DVASSERT(shaderValid, "Shader is invalid. Check log for details.");

        if (shaderValid)
        {
            res |= variant.second->shader->GetRequiredVertexFormat();
        }
    }
    return res;
}

MaterialBufferBinding* NMaterial::GetConstBufferBinding(UniquePropertyLayout propertyLayout)
{
    MaterialBufferBinding* res = NMaterialDetail::GetValuePtr(localConstBuffers, propertyLayout);

    if ((res == nullptr) && (parent != nullptr) && (!NeedLocalOverride(propertyLayout)))
    {
        res = parent->GetConstBufferBinding(propertyLayout);
    }

    return res;
}

NMaterialProperty* NMaterial::GetMaterialProperty(const FastName& propName)
{
    NMaterialProperty* res = NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName);

    if ((res == nullptr) && (parent != nullptr))
    {
        res = parent->GetMaterialProperty(propName);
    }

    return res;
}

Asset<Texture> NMaterial::GetEffectiveTexture(const FastName& slotName)
{
    MaterialTextureInfo* localInfo = NMaterialDetail::GetValuePtr(GetCurrentConfig().localTextures, slotName);

    if (localInfo)
    {
        if (localInfo->texture == nullptr)
        {
            Texture::PathKey key(localInfo->path);
            localInfo->texture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC, this);
        }
        return localInfo->texture;
    }

    if (parent != nullptr)
    {
        return parent->GetEffectiveTexture(slotName);
    }

    return nullptr;
}

void NMaterial::CollectLocalTextures(Set<MaterialTextureInfo*>& collection) const
{
    for (const auto& config : materialConfigs)
    {
        CollectConfigTextures(config, collection);
    }
}

void NMaterial::CollectActiveLocalTextures(Set<MaterialTextureInfo*>& collection) const
{
    CollectConfigTextures(GetCurrentConfig(), collection);
}

bool NMaterial::ContainsTexture(const Asset<Texture>& texture) const
{
    for (const auto& config : materialConfigs)
    {
        for (const auto& lc : config.localTextures)
        {
            if (lc.second->texture == texture)
                return true;
        }
    }

    return false;
}

const UnorderedMap<FastName, MaterialTextureInfo*>& NMaterial::GetLocalTextures() const
{
    return GetCurrentConfig().localTextures;
}

FXDescriptor::eType NMaterial::GetMaterialType() const
{
    return materialType;
}

void NMaterial::SetFXName(const FastName& fx)
{
    DVASSERT(materialType != FXDescriptor::TYPE_GLOBAL || !fx.IsValid());

    GetMutableCurrentConfig().fxName = fx;
    InvalidateRenderVariants();
}

const FastName& NMaterial::GetEffectiveFXName() const
{
    if ((!GetCurrentConfig().fxName.IsValid()) && (parent != nullptr))
    {
        return parent->GetEffectiveFXName();
    }
    return GetCurrentConfig().fxName;
}

const FastName& NMaterial::GetLocalFXName() const
{
    return GetCurrentConfig().fxName;
}

bool NMaterial::HasLocalFXName() const
{
    return GetCurrentConfig().fxName.IsValid();
}

const FastName& NMaterial::GetQualityGroup()
{
    if ((!qualityGroup.IsValid()) && (parent != nullptr))
    {
        return parent->GetQualityGroup();
    }
    return qualityGroup;
}

void NMaterial::SetQualityGroup(const FastName& quality)
{
    qualityGroup = quality;
}

void NMaterial::AddProperty(const FastName& propName, const float32* propData, rhi::ShaderProp::Type type, uint32 arraySize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    MaterialConfig& config = GetMutableCurrentConfig();

    DVASSERT(NMaterialDetail::GetValuePtr(config.localProperties, propName) == nullptr);

    NMaterialProperty* prop = new NMaterialProperty();
    prop->name = propName;
    prop->type = type;
    prop->arraySize = arraySize;
    prop->data.reset(new float[ShaderDescriptor::CalculateDataSize(type, arraySize)]);
    prop->SetPropertyValue(propData);
    config.localProperties[propName] = prop;

    InvalidateBufferBindings();
}

void NMaterial::RemoveProperty(const FastName& propName)
{
    MaterialConfig& config = GetMutableCurrentConfig();
    NMaterialProperty* prop = NMaterialDetail::GetValuePtr(config.localProperties, propName);

    DVASSERT(prop != nullptr);

    config.localProperties.erase(propName);
    SafeDelete(prop);

    InvalidateBufferBindings();
}

void NMaterial::SetPropertyValue(const FastName& propName, const float32* propData)
{
    NMaterialProperty* prop = NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName);

    DVASSERT(prop != nullptr);

    prop->SetPropertyValue(propData);
}

bool NMaterial::HasLocalProperty(const FastName& propName)
{
    return NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName) != nullptr;
}

rhi::ShaderProp::Type NMaterial::GetLocalPropType(const FastName& propName)
{
    NMaterialProperty* prop = NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName);

    DVASSERT(prop != nullptr);

    return prop->type;
}

const float32* NMaterial::GetLocalPropValue(const FastName& propName)
{
    NMaterialProperty* prop = NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName);

    DVASSERT(prop != nullptr);

    return prop->data.get();
}

uint32 NMaterial::GetLocalPropArraySize(const FastName& propName)
{
    NMaterialProperty* prop = NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName);

    DVASSERT(prop != nullptr);

    return prop->arraySize;
}

const float32* NMaterial::GetEffectivePropValue(const FastName& propName)
{
    NMaterialProperty* prop = NMaterialDetail::GetValuePtr(GetCurrentConfig().localProperties, propName);

    if (prop)
        return prop->data.get();

    if (parent)
        return parent->GetEffectivePropValue(propName);

    return nullptr;
}

const DAVA::UnorderedMap<DAVA::FastName, NMaterialProperty*>& NMaterial::GetLocalProperties() const
{
    return GetCurrentConfig().localProperties;
}

void NMaterial::AddTexture(const FastName& slotName, const Asset<Texture>& texture)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    AssetManager* assetManager = GetEngineContext()->assetManager;

    MaterialConfig& config = GetMutableCurrentConfig();

    DVASSERT(NMaterialDetail::GetValuePtr(config.localTextures, slotName) == nullptr);

    MaterialTextureInfo* texInfo = new MaterialTextureInfo();
    assetManager->RegisterListener(texture, this);
    texInfo->texture = texture;
    texInfo->path = texture->GetPathname();
    config.localTextures[slotName] = texInfo;
    InvalidateTextureBindings();
}

void NMaterial::RemoveTexture(const FastName& slotName)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;

    MaterialConfig& config = GetMutableCurrentConfig();
    MaterialTextureInfo* texInfo = NMaterialDetail::GetValuePtr(config.localTextures, slotName);

    DVASSERT(texInfo != nullptr);

    config.localTextures.erase(slotName);
    assetManager->UnregisterListener(texInfo->texture, this);
    SafeDelete(texInfo);
    InvalidateTextureBindings();
}

void NMaterial::SetTexture(const FastName& slotName, const Asset<Texture>& texture)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    MaterialTextureInfo* texInfo = NMaterialDetail::GetValuePtr(GetCurrentConfig().localTextures, slotName);

    DVASSERT(texture != nullptr); //use RemoveTexture to remove texture!
    DVASSERT(texInfo != nullptr); //use AddTexture to add texture!

    if (texInfo->texture != texture)
    {
        assetManager->UnregisterListener(texInfo->texture, this);
        assetManager->RegisterListener(texture, this);
        texInfo->path = texture->GetPathname();
    }

    InvalidateTextureBindings();
}

bool NMaterial::HasLocalTexture(const FastName& slotName)
{
    const MaterialConfig& config = GetCurrentConfig();
    return config.localTextures.find(slotName) != config.localTextures.end();
}

Asset<Texture> NMaterial::GetLocalTexture(const FastName& slotName)
{
    DVASSERT(HasLocalTexture(slotName));

    MaterialTextureInfo* texInfo = NMaterialDetail::GetValuePtr(GetCurrentConfig().localTextures, slotName);

    if (texInfo->texture == nullptr)
    {
        Texture::PathKey key(texInfo->path);
        texInfo->texture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
    }

    return texInfo->texture;
}

void NMaterial::AddFlag(const FastName& flagName, int32 value)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    MaterialConfig& config = GetMutableCurrentConfig();

    DVASSERT(config.localFlags.find(flagName) == config.localFlags.end());

    config.localFlags[flagName] = value;
    InvalidateRenderVariants();
}

void NMaterial::RemoveFlag(const FastName& flagName)
{
    MaterialConfig& config = GetMutableCurrentConfig();

    DVASSERT(config.localFlags.find(flagName) != config.localFlags.end());

    config.localFlags.erase(flagName);
    InvalidateRenderVariants();
}

void NMaterial::SetFlag(const FastName& flagName, int32 value)
{
    MaterialConfig& config = GetMutableCurrentConfig();

    DVASSERT(config.localFlags.find(flagName) != config.localFlags.end());

    if (config.localFlags[flagName] != value)
    {
        config.localFlags[flagName] = value;
        InvalidateRenderVariants();
    }
}

int32 NMaterial::GetEffectiveFlagValue(const FastName& flagName)
{
    MaterialConfig& config = GetMutableCurrentConfig();
    auto it = config.localFlags.find(flagName);
    if (it != config.localFlags.end())
        return it->second;
    else if (parent)
        return parent->GetEffectiveFlagValue(flagName);
    return 0;
}

int32 NMaterial::GetLocalFlagValue(const FastName& flagName)
{
    MaterialConfig& config = GetMutableCurrentConfig();

    DVASSERT(config.localFlags.find(flagName) != config.localFlags.end());

    return config.localFlags[flagName];
}

bool NMaterial::HasLocalFlag(const FastName& flagName)
{
    MaterialConfig& config = GetMutableCurrentConfig();
    return config.localFlags.find(flagName) != config.localFlags.end();
}

bool NMaterial::NeedLocalOverride(UniquePropertyLayout propertyLayout)
{
    MaterialConfig& config = GetMutableCurrentConfig();
    for (auto& descr : ShaderDescriptor::GetProps(propertyLayout))
    {
        if (NMaterialDetail::GetValuePtr(config.localProperties, descr.uid) != nullptr)
            return true;
    }
    return false;
}

void NMaterial::SetParent(NMaterial* _parent)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(_parent != this);

    if (parent == _parent)
        return;

    if (parent)
    {
        parent->RemoveChildMaterial(this);
        SafeRelease(parent);
    }

    parent = _parent;
    sortingKey = uint32(uint64(parent));

    if (parent)
    {
        SafeRetain(parent);
        parent->AddChildMaterial(this);
    }

    InvalidateRenderVariants();
    NMaterialManager::Instance().UpdateMaterialHierarchy(this);
}

NMaterial* NMaterial::GetParent() const
{
    return parent;
}

NMaterial* NMaterial::GetTopLevelParent()
{
    NMaterial* result = this;
    while (result->GetParent() != nullptr)
    {
        result = result->GetParent();
    }
    return result;
}

const Vector<NMaterial*>& NMaterial::GetChildren() const
{
    return children;
}

void NMaterial::AddChildMaterial(NMaterial* material)
{
    DVASSERT(material);
    children.push_back(material);
}

void NMaterial::RemoveChildMaterial(NMaterial* material)
{
    bool res = FindAndRemoveExchangingWithLast(children, material);
    DVASSERT(res);
}

//Configs managment
uint32 NMaterial::GetConfigCount() const
{
    return static_cast<uint32>(materialConfigs.size());
}

const FastName& NMaterial::GetCurrentConfigName() const
{
    return GetCurrentConfig().name;
}

void NMaterial::SetCurrentConfigName(const FastName& newName)
{
    GetMutableCurrentConfig().name = newName;
}

void NMaterial::SetCurrentConfigIndex(uint32 index)
{
    DVASSERT(index < materialConfigs.size());
    if (currentConfig != index)
    {
        currentConfig = index;
        InvalidateRenderVariants();
    }
}

void NMaterial::SetConfigName(uint32 index, const FastName& name)
{
    GetMutableConfig(index).name = name;
}

void NMaterial::ReleaseConfigTextures(uint32 index)
{
    MaterialConfig& config = GetMutableCurrentConfig();
    for (auto& tex : config.localTextures)
    {
        tex.second->texture.reset();
    }

    if (index == currentConfig)
        InvalidateTextureBindings();
}

const FastName& NMaterial::GetConfigName(uint32 index) const
{
    return GetConfig(index).name;
}

uint32 NMaterial::FindConfigByName(const FastName& name) const
{
    for (size_t i = 0, sz = materialConfigs.size(); i < sz; ++i)
    {
        if (materialConfigs[i].name == name)
            return static_cast<uint32>(i);
    }
    return static_cast<uint32>(materialConfigs.size());
}

void NMaterial::InsertConfig(uint32 index, const MaterialConfig& config)
{
    if (index < static_cast<uint32>(materialConfigs.size()))
    {
        materialConfigs.insert(materialConfigs.begin() + index, config);
    }
    else
    {
        materialConfigs.push_back(config);
    }
}

void NMaterial::RemoveConfig(uint32 index)
{
    DVASSERT(index < materialConfigs.size());
    DVASSERT(materialConfigs.size() > 1);
    materialConfigs.erase(materialConfigs.begin() + index);
    if (index == currentConfig)
    {
        InvalidateTextureBindings();
        InvalidateRenderVariants();
    }

    currentConfig = Min(currentConfig, static_cast<uint32>(materialConfigs.size()) - 1);
}

void NMaterial::InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer)
{
    if (parent && !NeedLocalOverride(propLayoutId))
        parent->InjectChildBuffer(propLayoutId, buffer);
    else
    {
        DVASSERT(NMaterialDetail::GetValuePtr(localConstBuffers, propLayoutId) == nullptr);
        localConstBuffers[propLayoutId] = buffer;
    }
}

void NMaterial::ClearLocalBuffers()
{
    for (auto& buffer : localConstBuffers)
    {
        rhi::DeleteConstBuffer(buffer.second->constBuffer);
        SafeDelete(buffer.second);
    }

    for (auto& variant : renderVariants)
    {
        // GFX_COMPLETE ????
        //variant.second->vertexConstBuffers.clear();
        //variant.second->fragmentConstBuffers.clear();

        variant.second->materialBufferBindings.clear();
    }
    localConstBuffers.clear();
}

void NMaterial::InvalidateBufferBindings()
{
#if TRACE_MATERIAL_REBUILDS
    if (!needRebuildBindings)
    {
        Logger::Info("InvalidateBufferBindings , material %s pointer %p", materialName.IsValid() ? materialName.c_str() : "", this);
        Logger::Info("call stack: \n %s", Debug::GetBacktraceString(20).c_str());
    }
#endif

    ClearLocalBuffers(); //RHI_COMPLETE - as local buffers can have binding for this property now just clear them all, later rethink to erase just buffers containing this property
    needRebuildBindings = true;
    for (auto& child : children)
        child->InvalidateBufferBindings();
}

void NMaterial::InvalidateTextureBindings()
{
    // reset existing handle?
    needRebuildTextures = true;
    for (auto& child : children)
        child->InvalidateTextureBindings();
}

void NMaterial::InvalidateRenderVariants()
{
#if TRACE_MATERIAL_REBUILDS
    if (!needRebuildVariants)
    {
        Logger::Info("InvalidateRenderVariants , material %s pointer %p", materialName.IsValid() ? materialName.c_str() : "", this);
        Logger::Info("call stack: \n %s", Debug::GetBacktraceString(20).c_str());
    }
#endif

    // release existing descriptor?
    ClearLocalBuffers(); // to avoid using incorrect buffers in certain situations (e.g chaning parent)
    needRebuildVariants = true;
    for (auto& child : children)
        child->InvalidateRenderVariants();
}

void NMaterial::PreCacheFX()
{
    UnorderedMap<FastName, int32> flags(16);
    CollectMaterialFlags(flags);
    NMaterialManager::Instance().MergeGlobalFlags(flags);

    FXAsset::Key key(GetEffectiveFXName(), QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()), std::move(flags));
    GetEngineContext()->assetManager->GetAsset<FXAsset>(key, AssetManager::SYNC);
}

void NMaterial::PreCacheFXWithFlags(const UnorderedMap<FastName, int32>& extraFlags, const FastName& extraFxName)
{
    UnorderedMap<FastName, int32> flags(16);
    CollectMaterialFlags(flags);
    NMaterialManager::Instance().MergeGlobalFlags(flags);
    for (auto& it : extraFlags)
    {
        if (it.second == 0)
            flags.erase(it.first);
        else
            flags[it.first] = it.second;
    }

    FXAsset::Key key(extraFxName.IsValid() ? extraFxName : GetEffectiveFXName(), QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()), std::move(flags));
    GetEngineContext()->assetManager->GetAsset<FXAsset>(key, AssetManager::SYNC);
}

void NMaterial::PreCacheFXVariations(const Vector<FastName>& fxNames, const Vector<FastName>& flags)
{
    uint32 flagsCount = static_cast<uint32>(flags.size());
    uint32 variations = 1u << flagsCount;
    for (const FastName& fxName : fxNames)
    {
        for (uint32 i = 0; i < variations; ++i)
        {
            UnorderedMap<FastName, int32> enabledFlags;
            for (uint32 f = 0; f < flagsCount; ++f)
            {
                enabledFlags[flags[f]] = static_cast<int32>((i & (1 << f)) != 0);
            }
            PreCacheFXWithFlags(enabledFlags, fxName);
        }
    }
}

void NMaterial::OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset)
{
    if (fxAsset == originalAsset)
    {
        fxAsset = std::dynamic_pointer_cast<FXAsset>(reloadedAsset);
        InvalidateRenderVariants();
    }
    else
    {
        Asset<Texture> texture = std::dynamic_pointer_cast<Texture>(reloadedAsset);
        if (texture != nullptr)
        {
            for (size_t i = 0; i < materialConfigs.size(); ++i)
            {
                MaterialConfig& config = materialConfigs[i];
                for (auto& node : config.localTextures)
                {
                    if (node.second->texture == originalAsset)
                    {
                        node.second->texture = texture;
                    }
                }
            }
            RebuildTextureBindings();
        }
        for (auto& variant : renderVariants)
        {
            if (variant.second->shader == originalAsset)
            {
                InvalidateRenderVariants();
                return;
            }
        }
    }
}

void NMaterial::RebuildRenderVariants()
{
    InvalidateBufferBindings();
    AssetManager* assetManager = GetEngineContext()->assetManager;

    UnorderedMap<FastName, int32> flags(16);
    CollectMaterialFlags(flags);
    NMaterialManager::Instance().MergeGlobalFlags(flags);

    FXAsset::Key fxKey(GetEffectiveFXName(), QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()), std::move(flags));
    fxAsset = assetManager->GetAsset<FXAsset>(fxKey, AssetManager::SYNC, this);

    const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();

    if (renderPassDescriptors.size() == 0)
    {
        // dragon: because I'm fucking sick and tired of Render2D-init crashing (when I don't even need it)
        return;
    }

    /*at least in theory flag changes can lead to changes in number of render passes*/
    activeVariantInstance = nullptr;
    activeVariantName = FastName();
    for (auto& variant : renderVariants)
    {
        delete variant.second;
    }
    renderVariants.clear();

    for (auto& variantDescr : renderPassDescriptors)
    {
        RenderVariantInstance* variant = new RenderVariantInstance();
        variant->renderLayer = variantDescr.renderLayer;
        variant->depthState = variantDescr.depthStencilState;
        variant->shader = variantDescr.shader;
        variant->cullMode = variantDescr.cullMode;
        variant->wireFrame = variantDescr.wireframe;
        variant->alphablend = variantDescr.hasBlend;
        variant->alphatest = (variantDescr.templateDefines.count(FastName("ALPHATEST")) != 0);
        renderVariants[variantDescr.passName] = variant;
        assetManager->RegisterListener(variant->shader, this);
    }

    activeVariantName = FastName();
    activeVariantInstance = nullptr;
    needRebuildVariants = false;
#if TRACE_MATERIAL_REBUILDS
    if (!needRebuildBindings)
    {
        Logger::Info("RebuildRenderVariants , material %s pointer %p", materialName.IsValid() ? materialName.c_str() : "", this);
        Logger::Info("call stack: \n %s", Debug::GetBacktraceString(20).c_str());
    }
#endif
    needRebuildBindings = true;
    needRebuildTextures = true;
}

void NMaterial::CollectMaterialFlags(UnorderedMap<FastName, int32>& target)
{
    if (parent)
        parent->CollectMaterialFlags(target);
    for (auto& it : GetCurrentConfig().localFlags)
    {
        if (it.second == 0) //ZERO is a special value that means flag is off - at least all shaders are consider it to be this right now
            target.erase(it.first);
        else
            target[it.first] = it.second;
    }
}

void NMaterial::CollectConfigTextures(const MaterialConfig& config, Set<MaterialTextureInfo*>& collection) const
{
    for (const auto& lc : config.localTextures)
    {
        const auto& path = lc.second->path;
        if (!path.IsEmpty())
        {
            collection.emplace(lc.second);
        }
    }
}

void NMaterial::RebuildBindings()
{
    InvalidateBufferBindings();

    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;
        Asset<ShaderDescriptor> currShader = (currRenderVariant != nullptr) ? currRenderVariant->shader : nullptr;
        if ((currShader == nullptr) || (currShader->IsValid() == false))
            continue;

        currRenderVariant->vertexConstBuffers.resize(currShader->GetVertexConstBuffersCount());
        currRenderVariant->fragmentConstBuffers.resize(currShader->GetFragmentConstBuffersCount());

        for (auto& bufferDescr : currShader->GetConstBufferDescriptors())
        {
            rhi::HConstBuffer bufferHandle;
            MaterialBufferBinding* bufferBinding = nullptr;
            //for static buffers resolve sharing and bindings
            if (bufferDescr.updateType == rhi::ShaderProp::SOURCE_MATERIAL)
            {
                bufferBinding = GetConstBufferBinding(bufferDescr.propertyLayoutId);
                //local buffers can contain buffer for corresponding layout if for example several passes us same buffer layout
                bool needLocalOverride = NeedLocalOverride(bufferDescr.propertyLayoutId) && (NMaterialDetail::GetValuePtr(localConstBuffers, bufferDescr.propertyLayoutId) == nullptr);
                //Create local buffer and build it's bindings if required;
                if ((bufferBinding == nullptr) || needLocalOverride)
                {
                    //create buffer
                    bufferBinding = new MaterialBufferBinding();

                    //create handles
                    if (bufferDescr.type == ConstBufferDescriptor::Type::Vertex)
                        bufferBinding->constBuffer = rhi::CreateVertexConstBuffer(currShader->GetPiplineState(), bufferDescr.targetSlot);
                    else
                        bufferBinding->constBuffer = rhi::CreateFragmentConstBuffer(currShader->GetPiplineState(), bufferDescr.targetSlot);

                    if (bufferBinding->constBuffer != rhi::InvalidHandle)
                    {
                        //if const buffer is InvalidHandle this means that whole const buffer was cut by shader compiler/linker
                        //it should not be updated but still can be shared as other shader variants can use it

                        //create bindings for this buffer
                        for (auto& propDescr : ShaderDescriptor::GetProps(bufferDescr.propertyLayoutId))
                        {
                            NMaterialProperty* prop = GetMaterialProperty(propDescr.uid);
                            if ((prop != nullptr)) //has property of the same type
                            {
                                DVASSERT(prop->type == propDescr.type);

                                // create property binding

                                bufferBinding->propBindings.emplace_back(propDescr.type,
                                                                         propDescr.bufferReg, propDescr.bufferRegCount, 0, prop);
                            }
                            else
                            {
                                //just set default property to const buffer
                                if (propDescr.type < rhi::ShaderProp::TYPE_FLOAT4)
                                {
                                    rhi::UpdateConstBuffer1fv(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.bufferRegCount, propDescr.defaultValue, ShaderDescriptor::CalculateDataSize(propDescr.type, 1));
                                }
                                else
                                {
                                    rhi::UpdateConstBuffer4fv(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.defaultValue, propDescr.bufferRegCount);
                                }
                            }
                        }
                    }

                    //store it locally or at parent
                    if (needLocalOverride || (!parent))
                    {
                        //buffer should be handled locally
                        DVASSERT(NMaterialDetail::GetValuePtr(localConstBuffers, bufferDescr.propertyLayoutId) == nullptr);
                        localConstBuffers[bufferDescr.propertyLayoutId] = bufferBinding;
                    }
                    else
                    {
                        //buffer can be propagated upward
                        parent->InjectChildBuffer(bufferDescr.propertyLayoutId, bufferBinding);
                    }
                }
                currRenderVariant->materialBufferBindings.push_back(bufferBinding);

                bufferHandle = bufferBinding->constBuffer;
            }

            else //if (bufferDescr.updateType == ConstBufferDescriptor::ConstBufferUpdateType::Static)
            {
                //for dynamic buffers just copy it's handle to corresponding slot
                bufferHandle = currShader->GetDynamicBuffer(bufferDescr.type, bufferDescr.targetSlot);
            }

            if (bufferHandle.IsValid())
            {
                if (bufferDescr.type == ConstBufferDescriptor::Type::Vertex)
                    currRenderVariant->vertexConstBuffers[bufferDescr.targetSlot] = bufferHandle;
                else
                    currRenderVariant->fragmentConstBuffers[bufferDescr.targetSlot] = bufferHandle;
            }
        }
    }

    needRebuildBindings = false;
}

void NMaterial::RebuildTextureBindings()
{
    InvalidateTextureBindings();

    uint32 anisotropyLevel = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Anisotropy>();
    anisotropyLevel = std::min(anisotropyLevel, rhi::DeviceCaps().maxAnisotropy);

    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;

        //release existing
        rhi::ReleaseTextureSet(currRenderVariant->textureSet);
        rhi::ReleaseSamplerState(currRenderVariant->samplerState);

        Asset<ShaderDescriptor> currShader = (currRenderVariant != nullptr) ? currRenderVariant->shader : nullptr;
        if ((currShader == nullptr) || (currShader->IsValid() == false))
            continue;

        rhi::TextureSetDescriptor textureDescr;
        rhi::SamplerState::Descriptor samplerDescr;

        // GFX_COMPLETE : seems there is a bug in this asser, should be lower, not lower and equal

        const rhi::ShaderSamplerList& fragmentSamplerList = currShader->GetFragmentSamplerList();
        textureDescr.fragmentTextureCount = static_cast<uint32>(fragmentSamplerList.size());
        DVASSERT(textureDescr.fragmentTextureCount <= rhi::MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT);

        samplerDescr.fragmentSamplerCount = static_cast<uint32>(fragmentSamplerList.size());
        DVASSERT(samplerDescr.fragmentSamplerCount <= rhi::MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT);

        currRenderVariant->dynamicTextureCount = 0;

        for (size_t i = 0, sz = textureDescr.fragmentTextureCount; i < sz; ++i)
        {
            RuntimeTextures::eRuntimeTextureSemantic textureSemantic = RuntimeTextures::GetRuntimeTextureSemanticByName(fragmentSamplerList[i].uid);
            if (textureSemantic == RuntimeTextures::TEXTURE_STATIC)
            {
                Asset<Texture> tex = GetEffectiveTexture(fragmentSamplerList[i].uid);

                if ((tex != nullptr) && (tex->textureType != fragmentSamplerList[i].type))
                    Logger::Warning("Invalid texture type (%d, required %d) assigned to slot %s", static_cast<int32>(tex->textureType), static_cast<int32>(fragmentSamplerList[i].type), fragmentSamplerList[i].uid.c_str());

                if (tex && (tex->textureType == fragmentSamplerList[i].type))
                {
                    textureDescr.fragmentTexture[i] = tex->handle;
                    samplerDescr.fragmentSampler[i] = tex->samplerState;
                }
                else
                {
                    textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(fragmentSamplerList[i].type);
                    samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetPinkTextureSamplerState(fragmentSamplerList[i].type);
                }
                uint32 maxAnisotropy = 16;
                if (tex != nullptr)
                {
                    maxAnisotropy = tex->maxAnisotropyLevel;
                }
                samplerDescr.fragmentSampler[i].anisotropyLevel = std::min(anisotropyLevel, maxAnisotropy);
            }
            else
            {
                textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetRuntimeTexture(textureSemantic);
                samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(textureSemantic);
            }

            for (uint32 k = 0; k < DynamicBindings::DYNAMIC_TEXTURE_COUNT; ++k)
            {
                if (fragmentSamplerList[i].uid == DynamicBindings::GetDynamicTextureName(static_cast<DynamicBindings::eTextureSemantic>(k)))
                {
                    DVASSERT(currRenderVariant->dynamicTextureCount < RenderVariantInstance::MAX_DYNAMIC_TEXTURE_PER_SHADER);

                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].index = i;
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].isVertex = 0;
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].semantic = static_cast<DynamicBindings::eTextureSemantic>(k);
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].type = fragmentSamplerList[i].type;

                    samplerDescr.fragmentSampler[i] = rhi::SamplerState::Descriptor::Sampler();
                    samplerDescr.fragmentSamplerCount++;
                    currRenderVariant->dynamicTextureCount++;
                }
            }

            DVASSERT(textureDescr.fragmentTexture[i].IsValid());
        }

        const rhi::ShaderSamplerList& vertexSamplerList = currShader->GetVertexSamplerList();
        textureDescr.vertexTextureCount = static_cast<uint32>(vertexSamplerList.size());
        DVASSERT(textureDescr.vertexTextureCount <= rhi::MAX_VERTEX_TEXTURE_SAMPLER_COUNT);

        samplerDescr.vertexSamplerCount = static_cast<uint32>(vertexSamplerList.size());
        DVASSERT(samplerDescr.vertexSamplerCount <= rhi::MAX_VERTEX_TEXTURE_SAMPLER_COUNT);

        for (size_t i = 0, sz = textureDescr.vertexTextureCount; i < sz; ++i)
        {
            RuntimeTextures::eRuntimeTextureSemantic textureSemantic = RuntimeTextures::GetRuntimeTextureSemanticByName(vertexSamplerList[i].uid);
            if (textureSemantic == RuntimeTextures::TEXTURE_STATIC)
            {
                Asset<Texture> tex = GetEffectiveTexture(vertexSamplerList[i].uid);

                if ((tex != nullptr) && (tex->textureType != vertexSamplerList[i].type))
                    Logger::Warning("Invalid texture type (%d, required %d) assigned to slot %s", static_cast<int32>(tex->textureType), static_cast<int32>(vertexSamplerList[i].type), vertexSamplerList[i].uid.c_str());

                if (tex && (tex->textureType == vertexSamplerList[i].type))
                {
                    textureDescr.vertexTexture[i] = tex->handle;
                    samplerDescr.vertexSampler[i] = tex->samplerState;
                }
                else
                {
                    textureDescr.vertexTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(vertexSamplerList[i].type);
                    samplerDescr.vertexSampler[i] = Renderer::GetRuntimeTextures().GetPinkTextureSamplerState(vertexSamplerList[i].type);
                }
            }
            else
            {
                textureDescr.vertexTexture[i] = Renderer::GetRuntimeTextures().GetRuntimeTexture(textureSemantic);
                samplerDescr.vertexSampler[i] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(textureSemantic);
            }

            for (uint32 k = 0; k < DynamicBindings::DYNAMIC_TEXTURE_COUNT; ++k)
            {
                if (vertexSamplerList[i].uid == DynamicBindings::GetDynamicTextureName(static_cast<DynamicBindings::eTextureSemantic>(k)))
                {
                    DVASSERT(currRenderVariant->dynamicTextureCount < RenderVariantInstance::MAX_DYNAMIC_TEXTURE_PER_SHADER);
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].index = i;
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].isVertex = 1;
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].semantic = static_cast<DynamicBindings::eTextureSemantic>(k);
                    currRenderVariant->dynamicTextureBindInfo[currRenderVariant->dynamicTextureCount].type = vertexSamplerList[i].type;

                    samplerDescr.vertexSamplerCount++;
                    currRenderVariant->dynamicTextureCount++;
                }
            }
        }

        textureDescr.textureDynamic = (currRenderVariant->dynamicTextureCount != 0);
        currRenderVariant->textureSet = rhi::AcquireTextureSet(textureDescr);
        currRenderVariant->samplerState = rhi::AcquireSamplerState(samplerDescr);
    }

    needRebuildTextures = false;
}

bool NMaterial::PreBuildMaterial(const FastName& passName)
{
    /*
    static Set<FastName> prebuildNames;
    if (prebuildNames.count(passName) == 0)
    {
        int ppos = 0;
        char buffer[2048] = { };
        prebuildNames.insert(passName);
        for (const FastName& n : prebuildNames)
            ppos += sprintf(buffer + ppos, "%s; ", n.c_str());
        Logger::Info("Used passes: %s", buffer);
    }
    // */
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    //shader rebuild first - as it sets needRebuildBindings and needRebuildTextures
    if (needRebuildVariants)
        RebuildRenderVariants();
    if (needRebuildBindings)
    {
#if TRACE_MATERIAL_REBUILDS
        static int pbc = 0;
        pbc++;
        Logger::Info("Rebuilding Bindings %d for pass %s , material %s pointer %p", pbc, passName.c_str(), materialName.IsValid() ? materialName.c_str() : "", this);
        Logger::Info("call stack: \n %s", Debug::GetBacktraceString(20).c_str());
#endif
        RebuildBindings();
    }

    if (needRebuildTextures)
        RebuildTextureBindings();

    bool res = (activeVariantInstance != nullptr) && (activeVariantInstance->shader != nullptr) && (activeVariantInstance->shader->IsValid());
    if (activeVariantName != passName)
    {
        auto it = renderVariants.find(passName); // [passName];
        if (it != renderVariants.end())
        {
            activeVariantName = passName;
            activeVariantInstance = it->second;

            res = (activeVariantInstance->shader != nullptr) && (activeVariantInstance->shader->IsValid());
        }
        else
        {
            res = false;
            // FastName fxName = GetEffectiveFXName();
            // Logger::Error("Failed to build material from `%s` for pass `%s`", fxName.c_str(), passName.c_str());
        }
    }
    return res;
}

NMaterial* NMaterial::Clone(FXDescriptor::eType newType)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    FXDescriptor::eType type = (newType == FXDescriptor::TYPE_COUNT) ? GetMaterialType() : newType;
    NMaterial* clonedMaterial = new NMaterial(type);
    clonedMaterial->materialName = materialName;
    clonedMaterial->qualityGroup = qualityGroup;

    clonedMaterial->materialConfigs.resize(materialConfigs.size());
    for (size_t i = 0, sz = materialConfigs.size(); i < sz; ++i)
    {
        clonedMaterial->materialConfigs[i] = materialConfigs[i];
    }

    clonedMaterial->currentConfig = currentConfig;
    clonedMaterial->SetParent(parent);

    // DataNode properties
    clonedMaterial->id = 0;
    clonedMaterial->scene = scene;
    clonedMaterial->isRuntime = isRuntime;

    return clonedMaterial;
}

void NMaterial::SaveConfigToArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext, bool forceNameSaving)
{
    const MaterialConfig& config = GetConfig(configId);
    if (config.fxName.IsValid())
        archive->SetString(NMaterialSerializationKey::FXName, config.fxName.c_str());

    if (config.name.IsValid())
    {
        if (forceNameSaving || config.name != NMaterialSerializationKey::DefaultConfigName)
        {
            archive->SetString(NMaterialSerializationKey::ConfigName, config.name.c_str());
        }
    }

    ScopedPtr<KeyedArchive> propertiesArchive(new KeyedArchive());
    for (auto it = config.localProperties.begin(), itEnd = config.localProperties.end(); it != itEnd; ++it)
    {
        NMaterialProperty* property = it->second;

        uint32 dataSize = ShaderDescriptor::CalculateDataSize(property->type, property->arraySize) * sizeof(float32);
        uint32 storageSize = sizeof(uint8) + sizeof(uint32) + dataSize;
        uint8* propertyStorage = new uint8[storageSize];

        memcpy(propertyStorage, &property->type, sizeof(uint8));
        memcpy(propertyStorage + sizeof(uint8), &property->arraySize, sizeof(uint32));
        memcpy(propertyStorage + sizeof(uint8) + sizeof(uint32), property->data.get(), dataSize);

        propertiesArchive->SetByteArray(it->first.c_str(), propertyStorage, storageSize);

        SafeDeleteArray(propertyStorage);
    }
    archive->SetArchive("properties", propertiesArchive);

    ScopedPtr<KeyedArchive> texturesArchive(new KeyedArchive());
    for (auto it = config.localTextures.begin(), itEnd = config.localTextures.end(); it != itEnd; ++it)
    {
        if (!NMaterialTextureName::IsRuntimeTexture(it->first) && !it->second->path.IsEmpty())
        {
            String textureRelativePath = it->second->path.GetRelativePathname(serializationContext->GetScenePath());
            if (textureRelativePath.size() > 0)
            {
                texturesArchive->SetString(it->first.c_str(), textureRelativePath);
            }
        }
    }
    archive->SetArchive("textures", texturesArchive);

    ScopedPtr<KeyedArchive> flagsArchive(new KeyedArchive());
    for (auto it = config.localFlags.begin(), itEnd = config.localFlags.end(); it != itEnd; ++it)
    {
        if (!NMaterialFlagName::IsRuntimeFlag(it->first))
            flagsArchive->SetInt32(it->first.c_str(), it->second);
    }
    archive->SetArchive("flags", flagsArchive);
}

void NMaterial::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DataNode::Save(archive, serializationContext);

    if (parent)
        archive->SetUInt64(NMaterialSerializationKey::ParentMaterialKey, parent->GetNodeID());

    if (materialName.IsValid())
        archive->SetString(NMaterialSerializationKey::MaterialName, materialName.c_str());

    if (qualityGroup.IsValid())
        archive->SetString(NMaterialSerializationKey::QualityGroup, qualityGroup.c_str());

    archive->SetUInt32(NMaterialSerializationKey::MaterialType, uint32(materialType));

    uint32 configsCount = static_cast<uint32>(materialConfigs.size());
    if (configsCount == 1)
    {
        //preserve old storage format
        SaveConfigToArchive(0, archive, serializationContext, false);
    }
    else
    {
        archive->SetUInt32(NMaterialSerializationKey::ConfigCount, configsCount);
        for (uint32 i = 0, sz = configsCount; i < sz; ++i)
        {
            ScopedPtr<KeyedArchive> configArchive(new KeyedArchive());
            SaveConfigToArchive(i, configArchive, serializationContext, true);
            archive->SetArchive(Format(NMaterialSerializationKey::ConfigArchive.c_str(), i), configArchive);
        }
    }
}

void NMaterial::LoadConfigFromArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (archive == nullptr)
    {
        Logger::Error("material %s has no archive for config %d", materialName.c_str(), configId);
        return;
    }

    MaterialConfig& config = GetMutableConfig(configId);
    if (archive->IsKeyExists(NMaterialSerializationKey::FXName))
    {
        config.fxName = FastName(archive->GetString(NMaterialSerializationKey::FXName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::ConfigName))
    {
        config.name = FastName(archive->GetString(NMaterialSerializationKey::ConfigName));
    }

    if (archive->IsKeyExists("properties"))
    {
        const KeyedArchive::UnderlyingMap& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint8) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();
            FastName propName = FastName(it->first);
            uint8 propType = *ptr;
            ptr += sizeof(uint8);
            uint32 propSize = *(reinterpret_cast<const uint32*>(ptr));
            ptr += sizeof(uint32);
            const float32* data = reinterpret_cast<const float32*>(ptr);

            NMaterialProperty* newProp = new NMaterialProperty();
            newProp->name = propName;
            newProp->type = rhi::ShaderProp::Type(propType);
            newProp->arraySize = propSize;
            newProp->data.reset(new float32[ShaderDescriptor::CalculateDataSize(newProp->type, newProp->arraySize)]);
            newProp->SetPropertyValue(data);
            config.localProperties[newProp->name] = newProp;
        }
    }

    if (archive->IsKeyExists("textures"))
    {
        const KeyedArchive::UnderlyingMap& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = texturesMap.begin(); it != texturesMap.end(); ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo* texInfo = new MaterialTextureInfo();
            texInfo->path = serializationContext->GetScenePath() + relativePathname;
            config.localTextures[FastName(it->first)] = texInfo;
        }
    }

    if (archive->IsKeyExists("flags"))
    {
        const KeyedArchive::UnderlyingMap& flagsMap = archive->GetArchive("flags")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
            config.localFlags[FastName(it->first)] = it->second->AsInt32();
        }
    }
}

void NMaterial::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    materialConfigs.clear();
    currentConfig = 0;
    DataNode::Load(archive, serializationContext);

    if (serializationContext->GetVersion() < RHI_SCENE_VERSION)
    {
        LoadOldNMaterial(archive, serializationContext);
        DVASSERT(materialConfigs.size() == 1);
        materialConfigs[0].name = NMaterialSerializationKey::DefaultConfigName;
        return;
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialName))
    {
        materialName = FastName(archive->GetString(NMaterialSerializationKey::MaterialName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialType))
    {
        materialType = FXDescriptor::eType(archive->GetUInt32(NMaterialSerializationKey::MaterialType, uint32(FXDescriptor::TYPE_LEGACY)));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialKey))
    {
        uint64 materialKey = archive->GetUInt64(NMaterialSerializationKey::MaterialKey);
        id = materialKey;
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists(NMaterialSerializationKey::ParentMaterialKey))
    {
        parentKey = archive->GetUInt64(NMaterialSerializationKey::ParentMaterialKey);
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists(NMaterialSerializationKey::QualityGroup))
    {
        qualityGroup = FastName(archive->GetString(NMaterialSerializationKey::QualityGroup).c_str());
    }

    uint32 configCount = archive->GetUInt32(NMaterialSerializationKey::ConfigCount, 1);
    materialConfigs.resize(configCount);
    if (configCount == 1)
    {
        LoadConfigFromArchive(0, archive, serializationContext);
    }
    else
    {
        for (uint32 i = 0; i < configCount; ++i)
            LoadConfigFromArchive(i, archive->GetArchive(Format(NMaterialSerializationKey::ConfigArchive.c_str(), i)), serializationContext);
    }

    if (materialConfigs.size() == 1 && !materialConfigs[0].name.IsValid())
    {
        materialConfigs[0].name = NMaterialSerializationKey::DefaultConfigName;
    }
}

void NMaterial::LoadOldNMaterial(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    materialConfigs.resize(1);
    /*the following stuff is for importing old NMaterial stuff*/

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialName))
    {
        materialName = FastName(archive->GetString(NMaterialSerializationKey::MaterialName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialKey))
    {
        uint64 materialKey = archive->GetUInt64(NMaterialSerializationKey::MaterialKey);
        id = materialKey;
    }

    int32 oldType = 0;
    if (archive->IsKeyExists("materialType"))
    {
        oldType = archive->GetInt32("materialType");
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists(NMaterialSerializationKey::ParentMaterialKey))
    {
        parentKey = archive->GetUInt64(NMaterialSerializationKey::ParentMaterialKey);
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists("materialGroup"))
    {
        qualityGroup = FastName(archive->GetString("materialGroup").c_str());
    }

    // don't load fxName from material instance (type = 2)
    if (archive->IsKeyExists("materialTemplate") && oldType != 2)
    {
        auto materialTemplate = archive->GetString("materialTemplate");
        materialConfigs[0].fxName = materialTemplate.empty() ? FastName() : FastName(materialTemplate);
    }

    if (archive->IsKeyExists("textures"))
    {
        const KeyedArchive::UnderlyingMap& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = texturesMap.begin();
             it != texturesMap.end();
             ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo* texInfo = new MaterialTextureInfo();
            texInfo->path = serializationContext->GetScenePath() + relativePathname;
            materialConfigs[0].localTextures[FastName(it->first)] = texInfo;
        }
    }

    if (archive->IsKeyExists("setFlags"))
    {
        const KeyedArchive::UnderlyingMap& flagsMap = archive->GetArchive("setFlags")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
            AddFlag(FastName(it->first), it->second->AsInt32());
        }
    }

    //NMaterial hell - for some reason property types were saved as GL_XXX defines O_o
    const uint32 originalTypesCount = 5;
    struct
    {
        uint32 originalType;
        rhi::ShaderProp::Type newType;
    } propertyTypeRemapping[originalTypesCount] =
    {
      { 0x1406 /*GL_FLOAT*/, rhi::ShaderProp::TYPE_FLOAT1 },
      { 0x8B50 /*GL_FLOAT_VEC2*/, rhi::ShaderProp::TYPE_FLOAT2 },
      { 0x8B51 /*GL_FLOAT_VEC3*/, rhi::ShaderProp::TYPE_FLOAT3 },
      { 0x8B52 /*GL_FLOAT_VEC4*/, rhi::ShaderProp::TYPE_FLOAT4 },
      { 0x8B5C /*GL_FLOAT_MAT4*/, rhi::ShaderProp::TYPE_FLOAT4X4 }
    };

    Array<FastName, 8> propertyFloat4toFloat3 =
    { {
    NMaterialParamName::PARAM_FOG_COLOR,
    NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SKY,
    NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SUN
    } };

    Array<FastName, 3> propertyFloat3toFloat4 =
    { { NMaterialParamName::PARAM_FLAT_COLOR,
        NMaterialParamName::PARAM_DECAL_TILE_COLOR,
        NMaterialParamName::PARAM_TREE_LEAF_COLOR_MUL } };

    Array<FastName, 1> propertyFloat1toFloat2 =
    {
      { NMaterialParamName::PARAM_DECAL_TILE_SCALE }
    };

    if (archive->IsKeyExists("properties"))
    {
        const KeyedArchive::UnderlyingMap& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint32) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();

            FastName propName = FastName(it->first);
            uint32 propType = *(reinterpret_cast<const uint32*>(ptr));
            ptr += sizeof(uint32);
            uint8 propSize = *(reinterpret_cast<const uint8*>(ptr));
            ptr += sizeof(uint8);
            const float32* data = reinterpret_cast<const float32*>(ptr);
            for (uint32 i = 0; i < originalTypesCount; i++)
            {
                if (propType == propertyTypeRemapping[i].originalType)
                {
                    if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT4)
                    {
                        if (std::find(propertyFloat4toFloat3.begin(), propertyFloat4toFloat3.end(), propName) != propertyFloat4toFloat3.end())
                        {
                            AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT3, 1);
                            continue;
                        }
                    }
                    else if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT3)
                    {
                        if (std::find(propertyFloat3toFloat4.begin(), propertyFloat3toFloat4.end(), propName) != propertyFloat3toFloat4.end())
                        {
                            float32 data4[4];
                            Memcpy(data4, data, 3 * sizeof(float32));
                            data4[3] = 1.f;

                            AddProperty(propName, data4, rhi::ShaderProp::TYPE_FLOAT4, 1);
                            continue;
                        }
                    }
                    else if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT1)
                    {
                        if (std::find(propertyFloat1toFloat2.begin(), propertyFloat1toFloat2.end(), propName) != propertyFloat1toFloat2.end())
                        {
                            Vector2 v2(*data, *data);
                            AddProperty(propName, v2.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
                            continue;
                        }
                    }

                    AddProperty(propName, data, propertyTypeRemapping[i].newType, 1);
                }
            }
        }
    }
}

const UnorderedMap<FastName, int32>& NMaterial::GetLocalFlags() const
{
    return GetCurrentConfig().localFlags;
}
}
