#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialStateDynamicTexturesInsp.h"
#include "Render/Material/FXCache.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
///////////////////////////////////////////////////////////////////////////
///// NMaterialStateDynamicTexturesInsp implementation

NMaterialStateDynamicTexturesInsp::NMaterialStateDynamicTexturesInsp()
{
    defaultTexture = Texture::CreatePink();
}

NMaterialStateDynamicTexturesInsp::~NMaterialStateDynamicTexturesInsp()
{
    SafeRelease(defaultTexture);
}

void NMaterialStateDynamicTexturesInsp::FindMaterialTexturesRecursive(NMaterial* material, Set<FastName>& ret, bool parents) const
{
    auto fxName = material->GetEffectiveFXName();
    if (fxName.IsValid())
    {
        UnorderedMap<FastName, int32> flags;
        material->CollectMaterialFlags(flags);

        // shader data
        FXDescriptor fxDescriptor = FXCache::GetFXDescriptor(fxName, flags, QualitySettingsSystem::Instance()->GetCurMaterialQuality(material->qualityGroup));
        for (auto& descriptor : fxDescriptor.renderPassDescriptors)
        {
            if ((descriptor.shader == nullptr) || !descriptor.shader->IsValid())
                continue;

            const rhi::ShaderSamplerList& fragmentSamplers = descriptor.shader->GetFragmentSamplerList();
            for (const auto& samp : fragmentSamplers)
            {
                if ((RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC) && (!DynamicBindings::IsDynamicTexture(samp.uid)))
                    ret.insert(samp.uid);
            }

            const rhi::ShaderSamplerList& vertexSamplers = descriptor.shader->GetVertexSamplerList();
            for (const auto& samp : vertexSamplers)
            {
                if ((RuntimeTextures::GetRuntimeTextureSemanticByName(samp.uid) == RuntimeTextures::TEXTURE_STATIC) && (!DynamicBindings::IsDynamicTexture(samp.uid)))
                    ret.insert(samp.uid);
            }
        }
    }
    else
    {
        // if fxName is not valid (e.g global material)
        // we just add all local textures
        const MaterialConfig& config = material->GetCurrentConfig();
        for (const auto& t : config.localTextures)
            ret.insert(t.first);
    }

    if (nullptr != material->GetParent() && parents)
        FindMaterialTexturesRecursive(material->GetParent(), ret, true);

    if (!parents)
    {
        for (NMaterial* child : material->GetChildren())
            FindMaterialTexturesRecursive(child, ret, false);
    }
}

InspInfoDynamic::DynamicData NMaterialStateDynamicTexturesInsp::Prepare(void* object, int filter) const
{
    NMaterial* material = static_cast<NMaterial*>(object);
    DVASSERT(material);

    Set<FastName> localData;
    FindMaterialTexturesRecursive(material, localData, true);
    FindMaterialTexturesRecursive(material, localData, false);
    localData.erase(NMaterialTextureName::TEXTURE_HEIGHTMAP);
    localData.erase(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION);

    if (filter > 0)
    {
        auto checkAndAdd = [&localData](const FastName& name) {
            if (0 == localData.count(name))
            {
                localData.insert(name);
            }
        };

        checkAndAdd(NMaterialTextureName::TEXTURE_ALBEDO);
        checkAndAdd(NMaterialTextureName::TEXTURE_NORMAL);
        checkAndAdd(NMaterialTextureName::TEXTURE_DETAIL);
        checkAndAdd(NMaterialTextureName::TEXTURE_LIGHTMAP);
        checkAndAdd(NMaterialTextureName::TEXTURE_DECAL);
        checkAndAdd(NMaterialTextureName::TEXTURE_CUBEMAP);
        checkAndAdd(NMaterialTextureName::TEXTURE_DECALMASK);
        checkAndAdd(NMaterialTextureName::TEXTURE_DECALTEXTURE);
    }

    Vector<FastName>* data = new Vector<FastName>();
    data->reserve(localData.size());
    data->insert(data->end(), localData.begin(), localData.end());

    std::stable_sort(data->begin(), data->end(), [](const FastName& l, const FastName& r) {
        return strcmp(l.c_str(), r.c_str()) < 0;
    });

    DynamicData ddata;
    ddata.object = object;
    ddata.data = std::shared_ptr<void>(data);
    return ddata;
}

Vector<FastName> NMaterialStateDynamicTexturesInsp::MembersList(const DynamicData& ddata) const
{
    Vector<FastName>* textures = static_cast<Vector<FastName>*>(ddata.data.get());
    DVASSERT(textures);
    return *(textures);
}

InspDesc NMaterialStateDynamicTexturesInsp::MemberDesc(const DynamicData& ddata, const FastName& textureName) const
{
    return InspDesc(textureName.c_str());
}

VariantType NMaterialStateDynamicTexturesInsp::MemberValueGet(const DynamicData& ddata, const FastName& textureName) const
{
    VariantType ret;

    Vector<FastName>* textures = static_cast<Vector<FastName>*>(ddata.data.get());
    DVASSERT(textures);

    NMaterial* material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    if (std::find(textures->begin(), textures->end(), textureName) != textures->end())
    {
        Texture* tex = material->GetEffectiveTexture(textureName);
        if (nullptr != tex)
        {
            ret.SetFilePath(tex->GetPathname());
        }
        else
        {
            ret.SetFilePath(FilePath());
        }
    }

    return ret;
}

void NMaterialStateDynamicTexturesInsp::MemberValueSet(const DynamicData& ddata, const FastName& textureName, const VariantType& value)
{
    VariantType ret;

    Vector<FastName>* textures = static_cast<Vector<FastName>*>(ddata.data.get());
    DVASSERT(textures);

    NMaterial* material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    if (std::find(textures->begin(), textures->end(), textureName) != textures->end())
    {
        if (value.type == VariantType::TYPE_NONE)
        {
            if (material->HasLocalTexture(textureName))
            {
                material->RemoveTexture(textureName);
            }
        }
        else
        {
            ScopedPtr<Texture> texture;

            FilePath texPath = value.AsFilePath();
            if (texPath == FilePath())
            {
                texture = SafeRetain(defaultTexture);
            }
            else
            {
                texture = Texture::CreateFromFile(texPath);
            }

            if (material->HasLocalTexture(textureName))
            {
                material->SetTexture(textureName, texture);
            }
            else
            {
                material->AddTexture(textureName, texture);
            }
        }
    }
}

int NMaterialStateDynamicTexturesInsp::MemberFlags(const DynamicData& ddata, const FastName& textureName) const
{
    int flags = 0;

    NMaterial* material = static_cast<NMaterial*>(ddata.object);
    DVASSERT(material);

    flags |= I_VIEW;

    if (material->HasLocalTexture(textureName))
    {
        flags |= I_EDIT;
    }

    return flags;
}
};
