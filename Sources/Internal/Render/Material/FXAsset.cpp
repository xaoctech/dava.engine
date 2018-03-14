#include "Render/Material/FXAsset.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"

#include <memory>

ENUM_DECLARE(DAVA::FXDescriptor::eType)
{
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_GLOBAL, "Global");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_COMMON, "Common");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_PARTICLE, "Particle");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_LANDSCAPE, "Landscape");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_SKY, "Sky");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_DECAL, "Decal");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_DECAL_VT, "VTDecal");
    ENUM_ADD_DESCR(DAVA::FXDescriptor::TYPE_LEGACY, "Legacy");
}

namespace DAVA
{
FXDescriptor::FXDescriptor(const FXDescriptor& other)
    : fxName(other.fxName)
    , materialType(other.materialType)
    , renderPassDescriptors(other.renderPassDescriptors)
{
}

FXDescriptor& FXDescriptor::operator=(const FXDescriptor& other)
{
    fxName = other.fxName;
    materialType = other.materialType;
    renderPassDescriptors = other.renderPassDescriptors;

    return *this;
}

FXDescriptor::FXDescriptor(FXDescriptor&& other)
    : fxName(std::move(other.fxName))
    , materialType(other.materialType)
    , renderPassDescriptors(std::move(other.renderPassDescriptors))
{
}

FXDescriptor& FXDescriptor::operator=(FXDescriptor&& other)
{
    fxName = std::move(other.fxName);
    materialType = other.materialType;
    renderPassDescriptors = std::move(other.renderPassDescriptors);

    return *this;
}

FXAsset::FXAsset(const Any& assetKey)
    : AssetBase(assetKey)
{
}

FXAsset::~FXAsset()
{
    descriptor.renderPassDescriptors.clear();
    GetEngineContext()->assetManager->UnregisterListener(this);
}

const Vector<RenderPassDescriptor>& FXAsset::GetPassDescriptors() const
{
    return descriptor.renderPassDescriptors;
}

void FXAsset::OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded)
{
    Asset<ShaderDescriptor> shader = std::dynamic_pointer_cast<ShaderDescriptor>(reloaded);
    DVASSERT(shader != nullptr);
    for (RenderPassDescriptor& descr : descriptor.renderPassDescriptors)
    {
        if (descr.shader == original)
        {
            descr.shader = shader;
        }
    }
}

void FXAsset::SetFXDescriptor(FXDescriptor&& descriptor_)
{
    DVASSERT(descriptor.renderPassDescriptors.empty() == true);
    AssetManager* assetManager = GetEngineContext()->assetManager;
    descriptor = std::move(descriptor_);
    for (RenderPassDescriptor& passDescr : descriptor.renderPassDescriptors)
    {
        assetManager->RegisterListener(passDescr.shader, this);
    }
}

} // namespace DAVA
