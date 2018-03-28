#include "FXCache.h"

#include "Base/GlobalEnum.h"

#include "Render/ShaderCache.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPassNames.h"

#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Concurrency/Mutex.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{
namespace FXCacheDetails
{
Map<Vector<size_t>, FXDescriptor> fxDescriptors;
Map<std::pair<FastName, FastName>, FXDescriptor> oldTemplateMap;

FXDescriptor defaultFX;
bool initialized = false;
Mutex fxCacheMutex;
}

namespace FXCache
{
const FXDescriptor& LoadFXFromClassicTemplate(const FastName& fxName, UnorderedMap<FastName, int32>& defines, const Vector<size_t>& key, const FastName& quality);

void Initialize()
{
    using namespace FXCacheDetails;

    DVASSERT(!initialized);
    initialized = true;

    UnorderedMap<FastName, int32> defFlags;
    defFlags[FastName("MATERIAL_TEXTURE")] = 1;

    RenderPassDescriptor defaultPass;
    defaultPass.passName = PASS_FORWARD;
    defaultPass.renderLayer = RENDER_LAYER_OPAQUE_ID;
    defaultPass.shaderFileName = FastName("~res:/Materials/Shaders/Default/materials");
    defaultPass.shader = ShaderDescriptorCache::GetShaderDescriptor(defaultPass.shaderFileName, defFlags);
    defaultPass.templateDefines[FastName("MATERIAL_TEXTURE")] = 1;

    defaultFX.renderPassDescriptors.clear();
    defaultFX.renderPassDescriptors.push_back(defaultPass);
}

void Uninitialize()
{
    Clear();
    FXCacheDetails::initialized = false;
}

void Clear()
{
    DVASSERT(FXCacheDetails::initialized);
    //RHI_COMPLETE
}

const FXDescriptor& GetFXDescriptor(const FastName& fxName, UnorderedMap<FastName, int32>& defines, const FastName& quality)
{
    using namespace FXCacheDetails;

    DVASSERT(initialized);

    if (!fxName.IsValid() || strlen(fxName.c_str()) == 0)
    {
        return FXCacheDetails::defaultFX;
    }

    Renderer::GetRuntimeFlags().AddToDefines(defines);

    Vector<size_t> key = ShaderDescriptorCache::BuildFlagsKey(fxName, defines);

    if (quality.IsValid()) //quality made as part of fx key
        key.push_back(ShaderDescriptorCache::GetUniqueFlagKey(quality));

    LockGuard<Mutex> guard(FXCacheDetails::fxCacheMutex);
    auto it = fxDescriptors.find(key);
    if (it != fxDescriptors.end())
        return it->second;

    // not found - load new
    return LoadFXFromClassicTemplate(fxName, defines, key, quality);
}

using ParseNodeFunction = void (*)(const YamlNode*, RenderPassDescriptor&, const FastName&);

const YamlNode* ExtractNode(const YamlNode* base, const char* nodeName, const YamlNode*& output)
{
    output = base->Get(nodeName);
    return output;
}

void ParseStencilNode(const YamlNode* stencilNode, RenderPassDescriptor& target, const FastName& fxName)
{
    if (stencilNode == nullptr)
        return;

    const YamlNode* node = nullptr;

    rhi::DepthStencilState::Descriptor& ds = target.depthStateDescriptor;

    if (ExtractNode(stencilNode, "ref", node))
    {
        uint8 refValue = static_cast<uint8>(node->AsInt32());
        ds.stencilBack.refValue = refValue;
        ds.stencilFront.refValue = refValue;
    }

    if (ExtractNode(stencilNode, "mask", node))
    {
        uint8 maskValue = static_cast<uint8>(node->AsInt32());
        ds.stencilBack.readMask = maskValue;
        ds.stencilBack.writeMask = maskValue;
        ds.stencilFront.readMask = maskValue;
        ds.stencilFront.writeMask = maskValue;
    }

    if (ExtractNode(stencilNode, "enabled", node))
        ds.stencilEnabled = node->AsBool();

    if (ExtractNode(stencilNode, "funcFront", node))
        ds.stencilFront.func = GetCmpFuncByName(node->AsString());

    if (ExtractNode(stencilNode, "funcBack", node))
        ds.stencilBack.func = GetCmpFuncByName(node->AsString());

    if (ExtractNode(stencilNode, "passFront", node))
        ds.stencilFront.depthStencilPassOperation = GetStencilOpByName(node->AsString());

    if (ExtractNode(stencilNode, "passBack", node))
        ds.stencilBack.depthStencilPassOperation = GetStencilOpByName(node->AsString());

    if (ExtractNode(stencilNode, "failFront", node))
        ds.stencilFront.failOperation = GetStencilOpByName(node->AsString());

    if (ExtractNode(stencilNode, "failBack", node))
        ds.stencilBack.failOperation = GetStencilOpByName(node->AsString());

    if (ExtractNode(stencilNode, "zFailFront", node))
        ds.stencilFront.depthFailOperation = GetStencilOpByName(node->AsString());

    if (ExtractNode(stencilNode, "zFailBack", node))
        ds.stencilBack.depthFailOperation = GetStencilOpByName(node->AsString());

    ds.stencilTwoSided = (memcmp(&ds.stencilBack, &ds.stencilFront, sizeof(rhi::DepthStencilState::Descriptor::StencilDescriptor)) != 0);
}

void ParseStateNode(const YamlNode* renderStateNode, RenderPassDescriptor& target, const FastName& fxName)
{
    if (renderStateNode == nullptr)
        return;

    const YamlNode* node = nullptr;
    const YamlNode* stencilNode = nullptr;

    if (ExtractNode(renderStateNode, "fillMode", node))
        target.wireframe = (node->AsString() == "FILLMODE_WIREFRAME");

    if (ExtractNode(renderStateNode, "blendEnabled", node))
        target.hasBlend = node->AsBool();

    if (ExtractNode(renderStateNode, "cullMode", node))
    {
        if (node->AsString() == "FACE_BACK")
            target.cullMode = rhi::CULL_CW;
        if (node->AsString() == "FACE_FRONT")
            target.cullMode = rhi::CULL_CCW;
        if (node->AsString() == "NONE")
            target.cullMode = rhi::CULL_NONE;
    }

    rhi::DepthStencilState::Descriptor& ds = target.depthStateDescriptor;

    if (ExtractNode(renderStateNode, "depthFunc", node))
        ds.depthFunc = GetCmpFuncByName(node->AsString());

    if (ExtractNode(renderStateNode, "depthWrite", node))
        ds.depthWriteEnabled = node->AsBool();

    if (ExtractNode(renderStateNode, "depthTest", node))
        ds.depthTestEnabled = node->AsBool();

    if (ExtractNode(renderStateNode, "stencil", stencilNode))
        ParseStencilNode(stencilNode, target, fxName);

    const YamlNode* stateFlagsNode = nullptr;
    if (ExtractNode(renderStateNode, "state", stateFlagsNode))
    {
        Vector<String> states;
        Split(stateFlagsNode->AsString(), "| ", states);
        for (String& state : states)
        {
            if (state == "STATE_BLEND")
            {
                target.hasBlend = true;
            }
            else if (state == "STATE_DEPTH_WRITE")
            {
                ds.depthWriteEnabled = true;
            }
            else if (state == "STATE_DEPTH_TEST")
            {
                ds.depthTestEnabled = true;
            }
            else if (state == "STATE_STENCIL_TEST")
            {
                ds.stencilEnabled = true;
            }
        }
    }
};

void ParseDefinesNode(const YamlNode* node, RenderPassDescriptor& target, const FastName& fxName)
{
    if (node == nullptr)
        return;

    for (uint32 k = 0; k < node->GetCount(); ++k)
    {
        const YamlNode* singleDefineNode = node->Get(k);
        target.templateDefines[FastName(singleDefineNode->AsString().c_str())] = 1;
    }
};

void ParseShaderNode(const YamlNode* node, RenderPassDescriptor& target, const FastName& fxName)
{
    if (node == nullptr)
        return;

    target.shaderFileName = node->AsFastName();
};

void ParseLayerNode(const YamlNode* node, RenderPassDescriptor& target, const FastName& fxName)
{
    if (node == nullptr)
        return;

    switch (node->GetType())
    {
    case YamlNode::eType::TYPE_STRING:
    {
        target.renderLayer = RenderLayer::GetLayerIDByName(node->AsFastName());
        break;
    }
    case YamlNode::eType::TYPE_ARRAY:
    {
        DVASSERT(node->GetCount() == 1, "Layers array should contain only one node");
        if (node->GetCount() == 1)
            target.renderLayer = RenderLayer::GetLayerIDByName(node->Get(0)->AsFastName());
        break;
    }
    default:
        Logger::Error("Render pass %s in material %s contains invalid Layer node", target.passName.c_str(), fxName.c_str());
    };
}

void ParseUniquePinsNode(const YamlNode* node, RenderPassDescriptor& target, const FastName& fxName)
{
    if (node == nullptr)
        return;

    for (uint32 k = 0; k < node->GetCount(); ++k)
    {
        const YamlNode* pinNode = node->Get(k);
        FastName pinName = FastName(pinNode->Get(0)->AsString().c_str());
        int32 pinValue = pinNode->Get(1)->AsInt32();
        target.templateDefines[pinName] = pinValue;
    }
};

void ParseFlowsNode(const YamlNode* node, RenderPassDescriptor& target, const FastName& fxName)
{
    if (node == nullptr)
        return;

    for (uint32 i = 0; i < node->GetCount(); ++i)
    {
        const YamlNode* flow = node->Get(i);
        if (flow->GetType() == YamlNode::eType::TYPE_STRING)
        {
            int32 index = -1;
            String flowName = flow->AsString();
            if (flowName == "all")
            {
                for (uint32 i = uint32(RenderFlow::FirstValid); i < uint32(RenderFlow::Count); ++i)
                    target.supportsRenderFlow[i] = true;
            }
            else if (flowName == "HDR")
            {
                target.supportsRenderFlow[uint32(RenderFlow::HDRForward)] = true;
                target.supportsRenderFlow[uint32(RenderFlow::HDRDeferred)] = true;
            }
            else if (flowName == "LDR")
            {
                target.supportsRenderFlow[uint32(RenderFlow::LDRForward)] = true;
            }
            else if (flowName == "TileBasedHDR")
            {
                target.supportsRenderFlow[uint32(RenderFlow::TileBasedHDRForward)] = true;
                target.supportsRenderFlow[uint32(RenderFlow::TileBasedHDRDeferred)] = true;
            }
            else if (GlobalEnumMap<RenderFlow>::Instance()->ToValue(flowName.c_str(), index))
            {
                target.supportsRenderFlow[index] = true;
            }
            else
            {
                Logger::Error("Render pass %s in material %s contains invalid render flow id: %s. Supported identifiers are: "
                              "[\"all\", \"HDR\", \"HDR\", \"TileBasedHDR\", \"LDRForward\", \"HDRForward\", \"HDRDeferred\", \"TileBasedHDRForward\", \"TileBasedHDRDeferred\"]",
                              target.passName.c_str(), fxName.c_str(), flowName.c_str());
                DVASSERT(0, "Invalid render flow id. See log for details");
            }
        }
    }
}

static const Vector<std::pair<ParseNodeFunction, String>> NodeParsingFunctions = {
    { ParseStateNode, "RenderState" },
    { ParseDefinesNode, "UniqueDefines" },
    { ParseShaderNode, "Shader" },
    { ParseLayerNode, "Layers" },
    { ParseUniquePinsNode, "UniquePins" },
    { ParseFlowsNode, "SupportedRenderFlows" },
    { ParseStencilNode, "Stencil" },
};

void ProcessRenderPassNode(const YamlNode* renderPassNode, const String& explicitName, RenderPassDescriptor& passDescriptor, const FastName& fxName)
{
    if (explicitName.empty())
    {
        const YamlNode* node = nullptr;
        if (ExtractNode(renderPassNode, "Name", node))
            passDescriptor.passName = node->AsFastName();
    }
    else
    {
        passDescriptor.passName = FastName(explicitName.c_str());
    }

    for (const auto& p : NodeParsingFunctions)
    {
        (p.first)(renderPassNode->Get(p.second), passDescriptor, fxName);
    }
}

void ValidatePassRenderFlows(RenderPassDescriptor& passDescriptor, const FastName& fxName)
{
    if (std::count(std::begin(passDescriptor.supportsRenderFlow), std::end(passDescriptor.supportsRenderFlow), true) == 0)
    {
        Logger::Warning("Render pass %s in material %s does not have any supported render flows. All flows will be enabled", passDescriptor.passName.c_str(), fxName.c_str());
        std::fill(std::begin(passDescriptor.supportsRenderFlow), std::end(passDescriptor.supportsRenderFlow), true);
    }

    if (!passDescriptor.shaderFileName.IsValid())
    {
        Logger::Error("Render pass %s in material %s does not contain link to shader file", passDescriptor.passName.c_str(), fxName.c_str());
        DVASSERT(0, "Render pass does not contain link to shader file. See log for details");
    }
}

const FXDescriptor& LoadClassicTempalte(const FastName& fxName, const FastName& quality)
{
    using namespace FXCacheDetails;

    auto oldTemplate = oldTemplateMap.find(std::make_pair(fxName, quality));
    if (oldTemplate != oldTemplateMap.end())
        return oldTemplate->second;

    FilePath fxPath(fxName.c_str());
    ScopedPtr<YamlParser> parser(YamlParser::Create(fxPath));

    if ((parser.get() == nullptr))
    {
        // GFX_COMPLETE - attempt to load from Materials2 instead of Materials
        String absolutePathName = fxPath.GetFrameworkPath();
        size_t materialsPos = absolutePathName.find("~res:/Materials/");
        if (materialsPos == 0)
        {
            absolutePathName = "~res:/Materials2/" + absolutePathName.substr(materialsPos + 16);
            parser = ScopedPtr<YamlParser>(YamlParser::Create(absolutePathName));
        }
    }

    YamlNode* rootNode = (parser) ? parser->GetRootNode() : nullptr;
    if (rootNode == nullptr)
    {
        Logger::Error("Can't load requested old-material-template-into-fx: %s", fxPath.GetAbsolutePathname().c_str());
        return FXCacheDetails::defaultFX;
    }

    const YamlNode* materialTemplateNode = nullptr;
    if (ExtractNode(rootNode, "MaterialTemplate", materialTemplateNode))
    {
        const YamlNode* qualityNode = nullptr;
        if (quality.IsValid())
        {
            qualityNode = materialTemplateNode->Get(quality.c_str());
        }
        if (qualityNode == nullptr)
        {
            if ((quality.c_str() == nullptr) || (strlen(quality.c_str()) == 0))
            {
                Logger::Warning("Quality not defined for template: %s, loading default one.", fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            }
            else
            {
                Logger::Error("Template: %s do not support quality %s - loading first one.", fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            }
            qualityNode = materialTemplateNode->Get(materialTemplateNode->GetCount() - 1);
        }
        ScopedPtr<YamlParser> parserTechnique(YamlParser::Create(qualityNode->AsString()));
        if (parserTechnique)
        {
            rootNode = parserTechnique->GetRootNode();
        }

        if (rootNode == nullptr)
        {
            Logger::Error("Can't load technique from template: %s with quality %s", fxPath.GetAbsolutePathname().c_str(), quality.c_str());
            return FXCacheDetails::defaultFX;
        }

        parser = parserTechnique;
    }

    RenderPassDescriptor baseDescriptor;
    baseDescriptor.cullMode = rhi::CULL_NONE;
    baseDescriptor.renderLayer = RENDER_LAYER_OPAQUE_ID;
    baseDescriptor.depthStateDescriptor.depthFunc = rhi::CMP_LESS;
    baseDescriptor.depthStateDescriptor.depthTestEnabled = false;
    baseDescriptor.depthStateDescriptor.depthWriteEnabled = false;
    baseDescriptor.wireframe = false;
    baseDescriptor.hasBlend = false;

    // now load render technique
    FXDescriptor result;
    result.fxName = fxName;
    result.materialType = NMaterial::TYPE_LEGACY;

    const YamlNode* renderTechniqueNode = rootNode->Get("RenderTechnique");
    if (renderTechniqueNode == nullptr)
    {
        const YamlNode* shaderNode = nullptr;
        if (ExtractNode(rootNode, "ForwardPassShader", shaderNode))
        {
            const String& shaderFileName = shaderNode->AsString();
            result.renderPassDescriptors.emplace_back(baseDescriptor);
            RenderPassDescriptor& passDescriptor = result.renderPassDescriptors.back();
            passDescriptor.passName = PASS_FORWARD;
            passDescriptor.shaderFileName = FastName(shaderFileName.c_str());
            std::fill(std::begin(passDescriptor.supportsRenderFlow), std::end(passDescriptor.supportsRenderFlow), true);
            return (oldTemplateMap[std::make_pair(fxName, quality)] = result);
        }
        else
        {
            Logger::Error("Material %s does not contains render technique node", fxName.c_str());
            return FXCacheDetails::defaultFX;
        }
    }

    const YamlNode* materialTypeNode = nullptr;
    if (ExtractNode(renderTechniqueNode, "MaterialType", materialTypeNode))
    {
        int32 value = -1;
        const String& typeStr = materialTypeNode->AsString();

        if (GlobalEnumMap<NMaterial::eType>::Instance()->ToValue(typeStr.c_str(), value))
        {
            result.materialType = NMaterial::eType(value);
        }
        else
        {
            Logger::Warning("[FXCache::GetFXDescriptor] invalid MaterialType: %s; FX: %s", typeStr.c_str(), fxName.c_str());
        }
    }

    for (const auto& p : NodeParsingFunctions)
    {
        (p.first)(renderTechniqueNode->Get(p.second), baseDescriptor, fxName);
    }

    Vector<std::pair<String, const YamlNode*>> renderPasses;

    // attempt to get passes from "Passes" node
    const YamlNode* passesNode = nullptr;
    if (ExtractNode(renderTechniqueNode, "Passes", passesNode))
    {
        result.renderPassDescriptors.reserve(passesNode->GetCount());
        for (uint32 i = 0; i < passesNode->GetCount(); ++i)
        {
            const String& passName = passesNode->GetItemKeyName(i);
            renderPasses.emplace_back(passName, passesNode->Get(i));
        }
    }

    if (strstr(fxName.c_str(), "PBS.material"))
        printf(".");

    // legacy - find passes by "RenderPass" entry
    result.renderPassDescriptors.reserve(renderTechniqueNode->GetCount());
    for (uint32 k = 0; k < renderTechniqueNode->GetCount(); ++k)
    {
        if (renderTechniqueNode->GetItemKeyName(k).substr(0, 10) == "RenderPass")
            renderPasses.emplace_back(String(), renderTechniqueNode->Get(k));
    }

    for (auto& p : renderPasses)
    {
        result.renderPassDescriptors.emplace_back();
        RenderPassDescriptor& passDescriptor = result.renderPassDescriptors.back();
        {
            passDescriptor.templateDefines = baseDescriptor.templateDefines;
            passDescriptor.renderLayer = baseDescriptor.renderLayer;
            passDescriptor.wireframe = baseDescriptor.wireframe;
            passDescriptor.depthStateDescriptor = baseDescriptor.depthStateDescriptor;
            passDescriptor.cullMode = baseDescriptor.cullMode;
            passDescriptor.shaderFileName = baseDescriptor.shaderFileName;
            memcpy(passDescriptor.supportsRenderFlow, baseDescriptor.supportsRenderFlow, sizeof(baseDescriptor.supportsRenderFlow));
        }
        ProcessRenderPassNode(p.second, p.first, passDescriptor, fxName);
        ValidatePassRenderFlows(passDescriptor, fxName);
    }

    return oldTemplateMap[std::make_pair(fxName, quality)] = result;
}

const FXDescriptor& LoadFXFromClassicTemplate(const FastName& fxName, UnorderedMap<FastName, int32>& defines, const Vector<size_t>& key, const FastName& quality)
{
    static rhi::CullMode invCullModes[3] = { rhi::CULL_NONE, rhi::CULL_CW, rhi::CULL_CCW };
    //the stuff below is old old legacy carried from RenderTechnique and NMaterialTemplate

    FXDescriptor target = LoadClassicTempalte(fxName, quality); //we copy it to new fxdescr as single template can be compiled to many descriptors
    target.defines = defines; //combine

    for (RenderPassDescriptor& pass : target.renderPassDescriptors)
    {
        bool shaderHasValidRenderFlow = pass.supportsRenderFlow[uint32(Renderer::GetCurrentRenderFlow())];

        // if shader does not support current flow, try to find all supported AND currently allowed flows
        for (uint32 i = uint32(RenderFlow::FirstValid); (shaderHasValidRenderFlow == false) && (i < uint32(RenderFlow::Count)); ++i)
        {
            RenderFlow flow = RenderFlow(i);
            if (Renderer::IsRenderFlowAllowed(flow) && pass.supportsRenderFlow[i])
                shaderHasValidRenderFlow = true;
        }

        if (shaderHasValidRenderFlow)
        {
            // ShaderDescriptor* baseShader = ShaderDescriptorCache::GetShaderDescriptor(pass.shaderFileName, {});

            UnorderedMap<FastName, int32> shaderDefines = defines;
            for (auto& templateDefine : pass.templateDefines)
                shaderDefines[templateDefine.first] = templateDefine.second;

            if (pass.hasBlend)
            {
                if (shaderDefines.find(NMaterialFlagName::FLAG_BLENDING) == shaderDefines.end())
                    shaderDefines[NMaterialFlagName::FLAG_BLENDING] = BLENDING_ALPHABLEND;
            }
            else
            {
                shaderDefines.erase(NMaterialFlagName::FLAG_BLENDING);
            }

            pass.shader = ShaderDescriptorCache::GetShaderDescriptor(pass.shaderFileName, shaderDefines);
            pass.depthStencilState = rhi::AcquireDepthStencilState(pass.depthStateDescriptor);

            //create alt states
            rhi::DepthStencilState::Descriptor reversDesc = pass.depthStateDescriptor;
            bool createReverseState = false;
            switch (reversDesc.depthFunc)
            {
            case rhi::CmpFunc::CMP_LESS:
                reversDesc.depthFunc = rhi::CmpFunc::CMP_GREATER;
                createReverseState = true;
                break;
            case rhi::CmpFunc::CMP_LESSEQUAL:
                reversDesc.depthFunc = rhi::CmpFunc::CMP_GREATEREQUAL;
                createReverseState = true;
                break;
            case rhi::CmpFunc::CMP_GREATER:
                reversDesc.depthFunc = rhi::CmpFunc::CMP_LESS;
                createReverseState = true;
                break;
            case rhi::CmpFunc::CMP_GREATEREQUAL:
                reversDesc.depthFunc = rhi::CmpFunc::CMP_LESSEQUAL;
                createReverseState = true;
                break;
            }

            if (createReverseState)
            {
                pass.invDepthStencilState = rhi::AcquireDepthStencilState(reversDesc);
            }
            else
            {
                pass.invDepthStencilState = pass.depthStencilState;
            }

            pass.invCullMode = invCullModes[pass.cullMode];
        }
    }

    return FXCacheDetails::fxDescriptors[key] = target;
}
}
}
