#pragma once

#include "Asset/Asset.h"
#include "Base/BaseTypes.h"
#include "Render/Renderer.h"
#include "Render/RHI/rhi_ShaderSource.h"
#include "Render/UniqueStateSet.h"
#include "Render/DynamicBindings.h"

namespace DAVA
{
using UniquePropertyLayout = UniqueHandle;

struct ConstBufferDescriptor
{
    enum class Type
    {
        Vertex,
        Fragment
    };

    Type type;
    rhi::ShaderProp::Source updateType;
    uint32 targetSlot;

    UniquePropertyLayout propertyLayoutId;
};

struct DynamicPropertyBinding
{
    rhi::ShaderProp::Type type;
    uint32 reg;
    uint32 regCount; //offset for props less than 1 reg size
    uint32 arraySize;
    pointer_size updateSemantic;
    rhi::HConstBuffer buffer;
    DynamicBindings::eUniformSemantic dynamicPropertySemantic;
};

class ShaderDescriptor : public AssetBase
{
public: //utility
    static const rhi::ShaderPropList& GetProps(UniquePropertyLayout layout);
    static uint32 CalculateRegsCount(rhi::ShaderProp::Type type, uint32 arraySize); //return in registers
    static uint32 CalculateDataSize(rhi::ShaderProp::Type type, uint32 arraySize); //return in float

public:
    struct Key
    {
        Key() = default;
        Key(const FastName& name, const UnorderedMap<FastName, int32>& inputDefines);

        FastName name;
        UnorderedMap<FastName, int32> defines;
        Vector<size_t> shaderKey;
        size_t shaderKeyHash = 0;
    };

    void UpdateDynamicParams();
    void ClearDynamicBindings();

    uint32 GetVertexConstBuffersCount();
    uint32 GetFragmentConstBuffersCount();

    rhi::HConstBuffer GetDynamicBuffer(ConstBufferDescriptor::Type type, uint32 index);
    inline rhi::HPipelineState GetPiplineState()
    {
        return piplineState;
    }

    uint32 GetRequiredVertexFormat()
    {
        return requiredVertexFormat;
    }

    const Vector<ConstBufferDescriptor>& GetConstBufferDescriptors() const
    {
        return constBuffers;
    }
    const rhi::ShaderSamplerList& GetFragmentSamplerList() const
    {
        return fragmentSamplerList;
    }
    const rhi::ShaderSamplerList& GetVertexSamplerList() const
    {
        return vertexSamplerList;
    }

    bool IsValid();

private:
    ShaderDescriptor(const Any& assetKey);
    ~ShaderDescriptor();

    void UpdateConfigFromSource(rhi::ShaderSource* vSource, rhi::ShaderSource* fSource);

    Vector<ConstBufferDescriptor> constBuffers;

    uint32 vertexConstBuffersCount, fragmentConstBuffersCount;
    Vector<DynamicPropertyBinding> dynamicPropertyBindings;

    Map<std::pair<ConstBufferDescriptor::Type, uint32>, rhi::HConstBuffer> dynamicBuffers;
    rhi::HPipelineState piplineState;

    uint32 requiredVertexFormat;

    rhi::ShaderSamplerList fragmentSamplerList;
    rhi::ShaderSamplerList vertexSamplerList;

    bool valid;
    friend class ShaderAssetLoader;

    DAVA_VIRTUAL_REFLECTION(ShaderDescriptor, AssetBase);
};

inline bool ShaderDescriptor::IsValid()
{
    return valid;
}

template <>
bool AnyCompare<ShaderDescriptor::Key>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<ShaderDescriptor::Key>;
};
