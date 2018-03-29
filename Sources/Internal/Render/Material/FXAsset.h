#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetListener.h"

#include "Render/Shader/ShaderDescriptor.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/RenderBase.h"

#include "Base/BaseTypes.h"

namespace DAVA
{
struct RenderPassDescriptor
{
    FastName passName;
    FastName shaderFileName;
    UnorderedMap<FastName, int> templateDefines = UnorderedMap<FastName, int>(8);
    Asset<ShaderDescriptor> shader;
    bool supportsRenderFlow[uint32(RenderFlow::Count)]{};
    rhi::DepthStencilState::Descriptor depthStateDescriptor;
    rhi::HDepthStencilState depthStencilState;
    eRenderLayerID renderLayer = RENDER_LAYER_INVALID_ID;
    rhi::CullMode cullMode = rhi::CULL_NONE;
    bool hasBlend = false;
    bool wireframe = false;
};

struct FXDescriptor
{
    FXDescriptor() = default;
    FXDescriptor(const FXDescriptor& other);
    FXDescriptor& operator=(const FXDescriptor& other);

    FXDescriptor(FXDescriptor&& other);
    FXDescriptor& operator=(FXDescriptor&& other);

    enum eType
    {
        TYPE_LEGACY = 0,

        TYPE_GLOBAL,
        TYPE_COMMON,
        TYPE_PARTICLE,
        TYPE_LANDSCAPE,
        TYPE_SKY,
        TYPE_DECAL,
        TYPE_DECAL_VT,

        TYPE_COUNT
    };

    FastName fxName;
    eType materialType = TYPE_COUNT;
    Vector<RenderPassDescriptor> renderPassDescriptors;
};

class FXAsset : public AssetBase, public AssetListener
{
    friend class FXAssetLoader;

public:
    struct Key
    {
        Key() = default;
        Key(const FastName& fxName, const FastName& quality, UnorderedMap<FastName, int32>&& inputDefines);

        FastName fxName;
        FastName quality;
        UnorderedMap<FastName, int32> defines;
        Vector<size_t> fxKey;
    };

    FXAsset(const Any& assetKey);
    ~FXAsset();

    const Vector<RenderPassDescriptor>& GetPassDescriptors() const;

    void OnAssetReloaded(const Asset<AssetBase>& original, const Asset<AssetBase>& reloaded) override;

private:
    void SetFXDescriptor(FXDescriptor&& descriptor);

    FXDescriptor descriptor;
    UnorderedMap<FastName, int32> defines = UnorderedMap<FastName, int32>(16);
};

template <>
bool AnyCompare<FXAsset::Key>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<FXAsset::Key>;
} // namespace DAVA
