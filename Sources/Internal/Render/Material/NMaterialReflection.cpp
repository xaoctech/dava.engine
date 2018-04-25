#include "Render/Material/NMaterialReflection.h"

#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/Private/Wrappers/ValueWrapperClassFn.h"
#include "Render/Texture.h"
#include "Render/Shader/ShaderDescriptor.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
NMaterialPropertyValueWrapper::NMaterialPropertyValueWrapper(const FastName& key)
    : key(key)
{
}

rhi::ShaderProp::Type NMaterialPropertyValueWrapper::GetDefaultPropertyType(const NMaterial* material) const
{
    Asset<FXAsset> fx = material->GetFXAsset();
    DVASSERT(fx != nullptr);

    for (const RenderPassDescriptor& descriptor : fx->GetPassDescriptors())
    {
        if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
            continue;

        for (const auto& buff : descriptor.shader->GetConstBufferDescriptors())
        {
            const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(buff.propertyLayoutId);
            auto it = std::find_if(std::begin(props), std::end(props), [this](const rhi::ShaderProp& prop)
                                   {
                                       return prop.uid == key;
                                   });
            if (it == std::end(props))
                continue;

            return it->type;
        }
    }

    // STREAMING_COMPLETE
    return rhi::ShaderProp::TYPE_FLOAT1;
    //DVASSERT(false);
}

const Type* NMaterialPropertyValueWrapper::GetType(const ReflectedObject& object) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    rhi::ShaderProp::Type type;
    if (material->HasEffectiveProperty(key) == false)
    {
        type = GetDefaultPropertyType(material);
    }
    else
    {
        type = material->GetEffectivePropType(key);
    }

    switch (type)
    {
    case rhi::ShaderProp::Type::TYPE_FLOAT1:
        return Type::Instance<float32>();
    case rhi::ShaderProp::Type::TYPE_FLOAT2:
        return Type::Instance<Vector2>();
    case rhi::ShaderProp::Type::TYPE_FLOAT3:
        return Type::Instance<Vector3>();
    case rhi::ShaderProp::Type::TYPE_FLOAT4:
        return Type::Instance<Vector4>();
    case rhi::ShaderProp::Type::TYPE_FLOAT4X4:
        return Type::Instance<Matrix4>();
    default:
        DVASSERT(false);
    }
}

bool NMaterialPropertyValueWrapper::IsReadonly(const ReflectedObject& object) const
{
    return false;
}

Any NMaterialPropertyValueWrapper::GetPropertyValue(rhi::ShaderProp::Type type, const float32* data) const
{
    switch (type)
    {
    case rhi::ShaderProp::Type::TYPE_FLOAT1:
        return Any(data[0]);
    case rhi::ShaderProp::Type::TYPE_FLOAT2:
        return Any(Vector2(data));
    case rhi::ShaderProp::Type::TYPE_FLOAT3:
        return Any(Vector3(data));
    case rhi::ShaderProp::Type::TYPE_FLOAT4:
        return Any(Vector4(data));
    case rhi::ShaderProp::Type::TYPE_FLOAT4X4:
        return Any(Matrix4(data[0], data[1], data[2], data[3],
                           data[4], data[5], data[6], data[7],
                           data[8], data[9], data[10], data[11],
                           data[12], data[13], data[14], data[15]));
    default:
        return Any();
    }
}

Any NMaterialPropertyValueWrapper::GetDefaultPropertyValue(const NMaterial* material) const
{
    Asset<FXAsset> fx = material->GetFXAsset();
    if (fx == nullptr)
        return Any();

    for (const RenderPassDescriptor& descriptor : fx->GetPassDescriptors())
    {
        if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
            continue;

        for (const auto& buff : descriptor.shader->GetConstBufferDescriptors())
        {
            const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(buff.propertyLayoutId);
            auto it = std::find_if(std::begin(props), std::end(props), [this](const rhi::ShaderProp& prop)
                                   {
                                       return prop.uid == key;
                                   });
            if (it == std::end(props))
                continue;

            return GetPropertyValue(it->type, it->defaultValue);
        }
    }
    return Any();
}

Any NMaterialPropertyValueWrapper::GetValue(const ReflectedObject& object) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    const float32* data = material->GetEffectivePropValue(key);
    if (data == nullptr || *data == 0) // should get default value from shader
    {
        return GetDefaultPropertyValue(material);
    }

    rhi::ShaderProp::Type type;
    type = material->GetEffectivePropType(key);
    return GetPropertyValue(type, data);
}

void NMaterialPropertyValueWrapper::SetProperty(NMaterial* material, const PropData& prop) const
{
    if (material->HasLocalProperty(key))
    {
        material->SetPropertyValue(key, prop.value);
    }
    else
    {
        material->AddProperty(key, prop.value, prop.type, prop.size);
    }
}

bool NMaterialPropertyValueWrapper::SetValue(const ReflectedObject& object, const Any& value) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    PropData prop;
    prop.size = 1;
    if (value.CanGet<float32>() == true)
    {
        const float32 val = value.Get<float32>();
        prop.value = &val;
        prop.type = rhi::ShaderProp::TYPE_FLOAT1;
    }
    else if (value.CanGet<Vector2>() == true)
    {
        Vector2 val = value.Get<Vector2>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT2;
    }
    else if (value.CanGet<Vector3>() == true)
    {
        Vector3 val = value.Get<Vector3>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT3;
    }
    else if (value.CanGet<Vector4>() == true)
    {
        Vector4 val = value.Get<Vector4>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT4;
    }
    else if (value.CanGet<Matrix4>() == true)
    {
        Matrix4 val = value.Get<Matrix4>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT4X4;
    }
    else
    {
        return false;
    }
    SetProperty(material, prop);
    return true;
}

bool NMaterialPropertyValueWrapper::SetValueWithCast(const ReflectedObject& object, const Any& value) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    PropData prop;
    prop.size = 1;

    if (value.CanCast<float32>() == true)
    {
        const float32 val = value.Cast<float32>();
        prop.value = &val;
        prop.type = rhi::ShaderProp::TYPE_FLOAT1;
    }
    else if (value.CanCast<Vector2>() == true)
    {
        Vector2 val = value.Cast<Vector2>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT2;
    }
    else if (value.CanCast<Vector3>() == true)
    {
        Vector3 val = value.Cast<Vector3>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT3;
    }
    else if (value.CanCast<Vector4>() == true)
    {
        Vector4 val = value.Cast<Vector4>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT4;
    }
    else if (value.CanCast<Matrix4>() == true)
    {
        Matrix4 val = value.Cast<Matrix4>();
        prop.value = val.data;
        prop.type = rhi::ShaderProp::TYPE_FLOAT4X4;
    }
    else
    {
        return false;
    }
    SetProperty(material, prop);
    return true;
}

ReflectedObject NMaterialPropertyValueWrapper::GetValueObject(const ReflectedObject& object) const
{
    return object;
}

// NMaterialTextureInfoValueWrapper

NMaterialTextureInfoValueWrapper::NMaterialTextureInfoValueWrapper(const FastName& key)
    : key(key)
{
}

void NMaterialTextureInfoValueWrapper::SetTexture(NMaterial* material, const FilePath& texturePath) const
{
    if (texturePath.IsEmpty())
    {
        material->RemoveTexture(key);
        return;
    }

    Texture::PathKey loadKey(texturePath);
    Asset<Texture> texture = GetEngineContext()->assetManager->GetAsset<Texture>(loadKey, AssetManager::SYNC);

    if (material->HasLocalTexture(key))
    {
        material->SetTexture(key, texture);
    }
    else
    {
        material->AddTexture(key, texture);
    }
}

const Type* NMaterialTextureInfoValueWrapper::GetType(const ReflectedObject& object) const
{
    return Type::Instance<FilePath>();
}

bool NMaterialTextureInfoValueWrapper::IsReadonly(const ReflectedObject& object) const
{
    return false;
}

Any NMaterialTextureInfoValueWrapper::GetValue(const ReflectedObject& object) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    MaterialTextureInfo* texture = material->GetEffectiveTextureInfo(key);
    if (texture == nullptr)
    {
        return Any(FilePath());
    }
    else
    {
        return Any(texture->path);
    }
}

bool NMaterialTextureInfoValueWrapper::SetValue(const ReflectedObject& object, const Any& value) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    if (value.CanGet<FilePath>() == false)
        return false;

    DVASSERT(value.Get<FilePath>().IsEqualToExtension(".tex"));

    SetTexture(material, value.Get<FilePath>());
    return true;
}

bool NMaterialTextureInfoValueWrapper::SetValueWithCast(const ReflectedObject& object, const Any& value) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    if (value.CanCast<FilePath>() == false)
        return false;

    Texture::PathKey loadKey(value.Cast<FilePath>());
    Asset<Texture> texture = GetEngineContext()->assetManager->GetAsset<Texture>(loadKey, AssetManager::SYNC);
    material->SetTexture(key, texture);
    return true;
}

ReflectedObject NMaterialTextureInfoValueWrapper::GetValueObject(const ReflectedObject& object) const
{
    //MaterialTextureInfo& texture = GetTextureInfo(object);
    NMaterial* material = object.GetPtr<NMaterial>();
    MaterialTextureInfo* texture = material->GetEffectiveTextureInfo(key);
    return ReflectedObject(&texture->path);
}

// NMaterialFlagValueWrapper

NMaterialFlagValueWrapper::NMaterialFlagValueWrapper(const FastName& key)
    : key(key)
{
}

void NMaterialFlagValueWrapper::SetFlag(NMaterial* material, int32 value) const
{
    if (material->HasLocalFlag(key))
    {
        material->SetFlag(key, value);
    }
    else
    {
        material->AddFlag(key, value);
    }
}

const Type* NMaterialFlagValueWrapper::GetType(const ReflectedObject& object) const
{
    return Type::Instance<int32>();
}

bool NMaterialFlagValueWrapper::IsReadonly(const ReflectedObject& object) const
{
    return false;
}

Any NMaterialFlagValueWrapper::GetValue(const ReflectedObject& object) const
{
    NMaterial* material = object.GetPtr<NMaterial>();
    return Any(material->GetEffectiveFlagValue(key));
}

bool NMaterialFlagValueWrapper::SetValue(const ReflectedObject& object, const Any& value) const
{
    if (value.CanGet<int32>() == false)
        return false;

    NMaterial* material = object.GetPtr<NMaterial>();
    int32 flag = value.Get<int32>();
    SetFlag(material, flag);
    return true;
}

bool NMaterialFlagValueWrapper::SetValueWithCast(const ReflectedObject& object, const Any& value) const
{
    if (value.CanCast<int32>() == false)
        return false;

    NMaterial* material = object.GetPtr<NMaterial>();
    int32 flag = value.Cast<int32>();
    SetFlag(material, flag);
    return true;
}

ReflectedObject NMaterialFlagValueWrapper::GetValueObject(const ReflectedObject& object) const
{
    return object;
}

// NMaterialStructureWrapper
Reflection NMaterialStructureWrapper::CreateReflection(const ReflectedObject& object, const String& key) const
{
    if (key == "Base")
    {
        return Reflection(
        object,
        &wrapper,
        &baseStructureWrapper,
        nullptr);
    }
    else if (key == "Flags")
    {
        return Reflection(
        object,
        &wrapper,
        &flagStructureWrapper,
        nullptr);
    }
    else if (key == "Properties")
    {
        return Reflection(
        object,
        &wrapper,
        &propertyStructureWrapper,
        nullptr);
    }
    else if (key == "Textures")
    {
        return Reflection(
        object,
        &wrapper,
        &textureInfoStructureWrapper,
        nullptr);
    }
    return Reflection();
}

bool NMaterialStructureWrapper::HasFields(const DAVA::ReflectedObject& object, const DAVA::ValueWrapper* vw) const
{
    return true;
}

size_t NMaterialStructureWrapper::GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const
{
    return 2;
}

Reflection NMaterialStructureWrapper::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (key.CanCast<String>() == false)
        return Reflection();

    ReflectedObject materialObject = vw->GetValueObject(object);
    if (materialObject.IsValid() == false)
    {
        return Reflection();
    }

    return CreateReflection(materialObject, key.Cast<String>());
}

Vector<Reflection::Field> NMaterialStructureWrapper::GetFields(const ReflectedObject& object,
                                                               const ValueWrapper* vw,
                                                               Reflection::MetaPredicate pred) const
{
    Vector<Reflection::Field> fields;
    ReflectedObject materialObject = vw->GetValueObject(object);
    if (materialObject.IsValid() == true)
    {
        for (const String& name : { "Base", "Flags", "Properties", "Textures" })
        {
            fields.emplace_back();
            Reflection::Field& f = fields.back();
            f.key = FastName(name);
            f.ref = CreateReflection(vw->GetValueObject(object), name);
        }
    }
    return fields;
}

// NMaterialPropertyStructureWrapper

const NMaterialPropertyValueWrapper& NMaterialPropertyStructureWrapper::GetOrCreateValueWrapper(const FastName& key) const
{
    if (propertyValueWrappers.find(key) != std::end(propertyValueWrappers))
        return propertyValueWrappers.at(key);

    propertyValueWrappers.emplace(key, key);
    return propertyValueWrappers.at(key);
}

bool NMaterialPropertyStructureWrapper::HasFields(const DAVA::ReflectedObject& object, const DAVA::ValueWrapper* vw) const
{
    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();
    if (material->GetLocalProperties().empty() == false)
        return true;

    Asset<FXAsset> fxAsset = material->GetFXAsset();
    if (fxAsset == nullptr)
        return false;

    const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();
    for (const RenderPassDescriptor& descriptor : renderPassDescriptors)
    {
        if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
            continue;

        for (const auto& buff : descriptor.shader->GetConstBufferDescriptors())
        {
            const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(buff.propertyLayoutId);
            for (const auto& prop : props)
            {
                if (prop.source == rhi::ShaderProp::SOURCE_AUTO)
                    continue;

                return true;
            }
        }
    }
    return false;
}

Reflection NMaterialPropertyStructureWrapper::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (key.CanCast<String>() == false)
        return Reflection();

    FastName fieldName = FastName(key.Cast<String>());
    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();
    Asset<FXAsset> fxAsset = material->GetFXAsset();
    if (fxAsset == nullptr) // global material
    {
        if (material->HasLocalProperty(fieldName) == false)
            return Reflection();

        const NMaterialPropertyValueWrapper& valueWrapper = GetOrCreateValueWrapper(fieldName);
        return Reflection(object,
                          &valueWrapper,
                          Reflection::GetDefaultStructureWrapper(),
                          nullptr);
    }
    else
    {
        const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();
        for (const RenderPassDescriptor& descriptor : renderPassDescriptors)
        {
            if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
                continue;

            for (const auto& buff : descriptor.shader->GetConstBufferDescriptors())
            {
                const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(buff.propertyLayoutId);
                for (const auto& prop : props)
                {
                    if (prop.source == rhi::ShaderProp::SOURCE_AUTO)
                        continue;

                    if (fieldName == prop.uid)
                    {
                        const NMaterialPropertyValueWrapper& valueWrapper = GetOrCreateValueWrapper(fieldName);
                        return Reflection(object,
                                          &valueWrapper,
                                          Reflection::GetDefaultStructureWrapper(),
                                          nullptr);
                    }
                }
            }
        }
    }

    return Reflection();
}

void NMaterialPropertyStructureWrapper::GetPropertiesRecursive(const ReflectedObject& object,
                                                               const NMaterial* material,
                                                               Set<FastName>& names,
                                                               Vector<Reflection::Field>& fields) const
{
    if (material == nullptr)
        return;

    Asset<FXAsset> fxAsset = material->GetFXAsset();
    if (fxAsset == nullptr) // global material
    {
        for (const auto& kv : material->GetLocalProperties())
        {
            if (names.find(kv.first) != names.end())
                continue;

            names.insert(kv.first);
            const NMaterialPropertyValueWrapper& valueWrapper = GetOrCreateValueWrapper(kv.first);
            fields.emplace_back();
            Reflection::Field& f = fields.back();
            f.key = kv.first;
            f.ref = Reflection(object,
                               &valueWrapper,
                               Reflection::GetDefaultStructureWrapper(),
                               nullptr);
        }
    }
    else
    {
        const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();
        for (const RenderPassDescriptor& descriptor : renderPassDescriptors)
        {
            if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
                continue;

            for (const auto& buff : descriptor.shader->GetConstBufferDescriptors())
            {
                const rhi::ShaderPropList& props = ShaderDescriptor::GetProps(buff.propertyLayoutId);
                for (const auto& prop : props)
                {
                    if (prop.source == rhi::ShaderProp::SOURCE_AUTO)
                        continue;

                    if (names.find(prop.uid) != names.end())
                        continue;

                    names.insert(prop.uid);
                    const NMaterialPropertyValueWrapper& valueWrapper = GetOrCreateValueWrapper(prop.uid);
                    fields.emplace_back();
                    Reflection::Field& f = fields.back();
                    f.key = prop.uid;
                    f.ref = Reflection(object,
                                       &valueWrapper,
                                       Reflection::GetDefaultStructureWrapper(),
                                       nullptr);
                }
            }
        }
    }
    GetPropertiesRecursive(object, material->GetParent(), names, fields);
}

Vector<Reflection::Field>
NMaterialPropertyStructureWrapper::GetFields(const ReflectedObject& object,
                                             const ValueWrapper* vw,
                                             Reflection::MetaPredicate pred) const
{
    Vector<Reflection::Field> fields;

    if (pred != nullptr && (*pred)(nullptr) == false)
        return fields;

    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();
    Set<FastName> s;
    GetPropertiesRecursive(object, material, s, fields);
    return fields;
}

bool NMaterialPropertyStructureWrapper::AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const
{
    return false;
}

bool NMaterialPropertyStructureWrapper::RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (key.CanCast<FastName>() == false)
        return false;

    FastName name = key.Cast<FastName>();
    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();
    material->RemoveProperty(name);
    return true;
}

// NMaterialTextureInfoStructureWrapper

const NMaterialTextureInfoValueWrapper& NMaterialTextureInfoStructureWrapper::GetOrCreateValueWrapper(const FastName& key) const
{
    if (textureInfoValueWrappers.find(key) != std::end(textureInfoValueWrappers))
        return textureInfoValueWrappers.at(key);

    textureInfoValueWrappers.emplace(key, key);
    return textureInfoValueWrappers.at(key);
}

bool NMaterialTextureInfoStructureWrapper::HasFields(const DAVA::ReflectedObject& object, const DAVA::ValueWrapper* vw) const
{
    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();
    if (material->GetLocalTextures().empty() == false)
        return true;

    Asset<FXAsset> fxAsset = material->GetFXAsset();
    if (fxAsset == nullptr)
        return false;

    const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();
    for (const RenderPassDescriptor& descriptor : renderPassDescriptors)
    {
        if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
            continue;

        const rhi::ShaderSamplerList& fragmentSamplers = descriptor.shader->GetFragmentSamplerList();
        for (const rhi::ShaderSampler& samp : fragmentSamplers)
        {
            if (RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC
                && DynamicBindings::IsDynamicTexture(samp.uid) == false)
            {
                return true;
            }
        }

        const rhi::ShaderSamplerList& vertexSamplers = descriptor.shader->GetVertexSamplerList();
        for (const rhi::ShaderSampler& samp : vertexSamplers)
        {
            if (RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC
                && DynamicBindings::IsDynamicTexture(samp.uid) == false)
            {
                return true;
            }
        }
    }
    return false;
}

Reflection NMaterialTextureInfoStructureWrapper::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (key.CanCast<String>() == false)
        return Reflection();

    FastName fieldName = FastName(key.Cast<String>());
    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();

    if (material->HasLocalTexture(fieldName) == true)
    {
        const NMaterialTextureInfoValueWrapper& valueWrapper = GetOrCreateValueWrapper(fieldName);
        return Reflection(object,
                          &valueWrapper,
                          Reflection::GetDefaultStructureWrapper(),
                          nullptr);
    }

    Asset<FXAsset> fxAsset = material->GetFXAsset();
    if (fxAsset == nullptr)
        return Reflection();

    const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();
    for (const RenderPassDescriptor& descriptor : renderPassDescriptors)
    {
        if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
            continue;

        const rhi::ShaderSamplerList& fragmentSamplers = descriptor.shader->GetFragmentSamplerList();
        for (const rhi::ShaderSampler& samp : fragmentSamplers)
        {
            if (samp.uid == fieldName
                && RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC
                && DynamicBindings::IsDynamicTexture(samp.uid) == false)
            {
                const NMaterialTextureInfoValueWrapper& valueWrapper = GetOrCreateValueWrapper(fieldName);
                return Reflection(object,
                                  &valueWrapper,
                                  Reflection::GetDefaultStructureWrapper(),
                                  nullptr);
            }
        }

        const rhi::ShaderSamplerList& vertexSamplers = descriptor.shader->GetVertexSamplerList();
        for (const rhi::ShaderSampler& samp : vertexSamplers)
        {
            if (samp.uid == fieldName
                && RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC
                && DynamicBindings::IsDynamicTexture(samp.uid) == false)
            {
                const NMaterialTextureInfoValueWrapper& valueWrapper = GetOrCreateValueWrapper(fieldName);
                return Reflection(object,
                                  &valueWrapper,
                                  Reflection::GetDefaultStructureWrapper(),
                                  nullptr);
            }
        }
    }

    return Reflection();
}

void NMaterialTextureInfoStructureWrapper::GetTextureInfoRecursive(const ReflectedObject& object,
                                                                   const NMaterial* material,
                                                                   Set<FastName>& names,
                                                                   Vector<Reflection::Field>& fields) const
{
    if (material == nullptr)
        return;

    Asset<FXAsset> fxAsset = material->GetFXAsset();
    for (const auto& kv : material->GetLocalTextures())
    {
        if (names.find(kv.first) != names.end())
            continue;

        const NMaterialTextureInfoValueWrapper& valueWrapper = GetOrCreateValueWrapper(kv.first);
        names.insert(kv.first);
        fields.emplace_back();
        Reflection::Field& f = fields.back();
        f.key = kv.first;
        f.ref = Reflection(object,
                           &valueWrapper,
                           Reflection::GetDefaultStructureWrapper(),
                           nullptr);
    }

    if (fxAsset != nullptr)
    {
        const Vector<RenderPassDescriptor>& renderPassDescriptors = fxAsset->GetPassDescriptors();
        for (const RenderPassDescriptor& descriptor : renderPassDescriptors)
        {
            if (descriptor.shader == nullptr || descriptor.shader->IsValid() == false)
                continue;

            const rhi::ShaderSamplerList& fragmentSamplers = descriptor.shader->GetFragmentSamplerList();
            for (const rhi::ShaderSampler& samp : fragmentSamplers)
            {
                if (RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC
                    && DynamicBindings::IsDynamicTexture(samp.uid) == false)
                {
                    if (names.find(samp.uid) != names.end())
                        continue;

                    names.insert(samp.uid);
                    fields.emplace_back();
                    Reflection::Field& f = fields.back();
                    const NMaterialTextureInfoValueWrapper& valueWrapper = GetOrCreateValueWrapper(samp.uid);
                    f.key = samp.uid;
                    f.ref = Reflection(object,
                                       &valueWrapper,
                                       Reflection::GetDefaultStructureWrapper(),
                                       nullptr);
                }
            }

            const rhi::ShaderSamplerList& vertexSamplers = descriptor.shader->GetVertexSamplerList();
            for (const rhi::ShaderSampler& samp : vertexSamplers)
            {
                if (RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC
                    && DynamicBindings::IsDynamicTexture(samp.uid) == false)
                {
                    if (names.find(samp.uid) != names.end())
                        continue;

                    names.insert(samp.uid);
                    fields.emplace_back();
                    Reflection::Field& f = fields.back();
                    const NMaterialTextureInfoValueWrapper& valueWrapper = GetOrCreateValueWrapper(samp.uid);
                    f.key = samp.uid;
                    f.ref = Reflection(object,
                                       &valueWrapper,
                                       Reflection::GetDefaultStructureWrapper(),
                                       nullptr);
                }
            }
        }
    }

    GetTextureInfoRecursive(object, material->GetParent(), names, fields);
}

Vector<Reflection::Field>
NMaterialTextureInfoStructureWrapper::GetFields(const ReflectedObject& object,
                                                const ValueWrapper* vw,
                                                Reflection::MetaPredicate pred) const
{
    Vector<Reflection::Field> fields;

    if (pred != nullptr && (*pred)(nullptr) == false)
        return fields;

    ReflectedObject reflectedMaterial = vw->GetValueObject(object);
    NMaterial* material = reflectedMaterial.GetPtr<NMaterial>();
    Set<FastName> s;
    GetTextureInfoRecursive(object, material, s, fields);
    return fields;
}

bool NMaterialTextureInfoStructureWrapper::AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const
{
    return false;
}

bool NMaterialTextureInfoStructureWrapper::RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    return false;
}

// NMaterialFlagStructureWrapper

Vector<FastName> NMaterialFlagStructureWrapper::flags =
{
  NMaterialFlagName::FLAG_VERTEXFOG,
  NMaterialFlagName::FLAG_FOG_LINEAR,
  NMaterialFlagName::FLAG_FOG_HALFSPACE,
  NMaterialFlagName::FLAG_FOG_HALFSPACE_LINEAR,
  NMaterialFlagName::FLAG_FOG_ATMOSPHERE,

  NMaterialFlagName::FLAG_FLATCOLOR,
  NMaterialFlagName::FLAG_FLATALBEDO,
  NMaterialFlagName::FLAG_TEXTURESHIFT,
  NMaterialFlagName::FLAG_TEXTURE0_ANIMATION_SHIFT,

  NMaterialFlagName::FLAG_WAVE_ANIMATION,
  NMaterialFlagName::FLAG_FAST_NORMALIZATION,

  NMaterialFlagName::FLAG_SPECULAR,
  NMaterialFlagName::FLAG_SEPARATE_NORMALMAPS,
  NMaterialFlagName::FLAG_TANGENT_SPACE_WATER_REFLECTIONS,
  NMaterialFlagName::FLAG_DEBUG_UNITY_Z_NORMAL,
  NMaterialFlagName::FLAG_DEBUG_Z_NORMAL_SCALE,
  NMaterialFlagName::FLAG_DEBUG_NORMAL_ROTATION,
  NMaterialFlagName::FLAG_TEST_OCCLUSION,
  NMaterialFlagName::FLAG_TILED_DECAL_MASK,
  NMaterialFlagName::FLAG_TILED_DECAL_ROTATION,
  NMaterialFlagName::FLAG_ALPHATESTVALUE,
  NMaterialFlagName::FLAG_ALPHASTEPVALUE,
  NMaterialFlagName::FLAG_FORCED_SHADOW_DIRECTION,

  NMaterialFlagName::FLAG_TRANSMITTANCE,
  NMaterialFlagName::FLAG_HARD_SKINNING,
  NMaterialFlagName::FLAG_SOFT_SKINNING,
  NMaterialFlagName::FLAG_USE_BAKED_LIGHTING,
  NMaterialFlagName::FLAG_ALBEDO_ALPHA_MASK,

  NMaterialFlagName::FLAG_VERTEX_BLEND_TEXTURES,
  NMaterialFlagName::FLAG_VERTEX_BLEND_4_TEXTURES,

  NMaterialFlagName::FLAG_ALBEDO_TINT_BLEND_MODE,

  NMaterialFlagName::FLAG_NORMAL_BLEND_MODE,

  NMaterialFlagName::FLAG_USE_DETAIL_NORMAL_AO,

  NMaterialFlagName::FLAG_BLEND_LANDSCAPE_HEIGHT,
  NMaterialFlagName::FLAG_RGBM_INPUT,
};

const NMaterialFlagValueWrapper& NMaterialFlagStructureWrapper::GetOrCreateValueWrapper(const FastName& key) const
{
    if (flagValueWrappers.find(key) != std::end(flagValueWrappers))
        return flagValueWrappers.at(key);

    flagValueWrappers.emplace(key, key);
    return flagValueWrappers.at(key);
}

bool NMaterialFlagStructureWrapper::HasFields(const DAVA::ReflectedObject& object, const DAVA::ValueWrapper* vw) const
{
    return true;
}

Reflection NMaterialFlagStructureWrapper::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (key.CanCast<String>() == false)
        return Reflection();

    FastName fieldName = FastName(key.Cast<String>());
    if (std::find(std::begin(flags), std::end(flags), fieldName) == std::end(flags))
        return Reflection();

    const NMaterialFlagValueWrapper& valueWrapper = GetOrCreateValueWrapper(fieldName);
    return Reflection(object,
                      &valueWrapper,
                      Reflection::GetDefaultStructureWrapper(),
                      nullptr);
}

Vector<Reflection::Field> NMaterialFlagStructureWrapper::GetFields(const ReflectedObject& object,
                                                                   const ValueWrapper* vw,
                                                                   Reflection::MetaPredicate pred) const
{
    Vector<Reflection::Field> fields;

    if (pred != nullptr && (*pred)(nullptr) == false)
        return fields;

    for (const auto& flag : flags)
    {
        const NMaterialFlagValueWrapper& valueWrapper = GetOrCreateValueWrapper(flag);
        fields.emplace_back();
        Reflection::Field& f = fields.back();
        f.key = flag;
        f.ref = Reflection(object,
                           &valueWrapper,
                           Reflection::GetDefaultStructureWrapper(),
                           nullptr);
    }
    return fields;
}

bool NMaterialFlagStructureWrapper::AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const
{
    return false;
}

bool NMaterialFlagStructureWrapper::RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    return false;
}

// NMaterialBaseStructureWrapper

Reflection NMaterialBaseStructureWrapper::CreateReflection(const ReflectedObject& object, const ValueWrapper* vw, const String& key) const
{
    ValueWrapper* activeVw = nullptr;
    if (key == "materialName")
    {
        static ValueWrapperClassFn<NMaterial, const FastName&, const FastName&>
        vw(&NMaterial::GetMaterialName,
           &NMaterial::SetMaterialName);
        activeVw = &vw;
    }
    else if (key == "fxName")
    {
        static ValueWrapperClassFn<NMaterial, const FastName&, const FastName&>
        vw(&NMaterial::GetEffectiveFXName,
           &NMaterial::SetFXName);
        activeVw = &vw;
    }
    else if (key == "qualityGroup")
    {
        static ValueWrapperClassFn<NMaterial, const FastName&, const FastName&>
        vw(&NMaterial::GetQualityGroup,
           &NMaterial::SetQualityGroup);
        activeVw = &vw;
    }
    else if (key == "materialType")
    {
        static ValueWrapperClassFn<NMaterial, FXDescriptor::eType, void*>
        vw(&NMaterial::GetMaterialType);
        activeVw = &vw;
    }
    else
    {
        return Reflection();
    }
    return Reflection(object, activeVw, nullptr, nullptr);
}

bool NMaterialBaseStructureWrapper::HasFields(const DAVA::ReflectedObject& object, const DAVA::ValueWrapper* vw) const
{
    return true;
}

Reflection NMaterialBaseStructureWrapper::GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const
{
    if (key.CanCast<String>() == false)
        return Reflection();

    String fieldName = key.Cast<String>();
    return CreateReflection(object, vw, fieldName);
}

Vector<Reflection::Field> NMaterialBaseStructureWrapper::GetFields(const ReflectedObject& object, const ValueWrapper* vw, Reflection::MetaPredicate pred) const
{
    Vector<Reflection::Field> fields;
    for (const String& key : { "fxName", "materialName", "qualityGroup", "materialType" })
    {
        fields.emplace_back();
        Reflection::Field& f = fields.back();
        f.key = FastName(key);
        f.ref = CreateReflection(object, vw, key);
    }
    return fields;
}

// -------------------------------------------------------

const Type* NMaterialValueWrapper::GetType(const ReflectedObject& object) const
{
    return Type::Instance<NMaterial>();
}

bool NMaterialValueWrapper::IsReadonly(const ReflectedObject& object) const
{
    return true;
}

Any NMaterialValueWrapper::GetValue(const ReflectedObject& object) const
{
    return Any();
}

bool NMaterialValueWrapper::SetValue(const ReflectedObject& object, const Any& value) const
{
    return false;
}

bool NMaterialValueWrapper::SetValueWithCast(const ReflectedObject& object, const Any& value) const
{
    return false;
}

ReflectedObject NMaterialValueWrapper::GetValueObject(const ReflectedObject& object) const
{
    return object;
}

Reflection CreateNMaterialReflection(NMaterial* material)
{
    static NMaterialValueWrapper valueWrapperDefault;

    ReflectedObject object(material, Type::Instance<NMaterial>());
    return Reflection(object, &valueWrapperDefault, nullptr, nullptr);
}
}
