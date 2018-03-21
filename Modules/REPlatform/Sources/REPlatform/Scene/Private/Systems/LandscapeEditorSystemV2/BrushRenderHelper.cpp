#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushRenderHelper.h"

#include <Base/BaseTypes.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialNames.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Render/RhiUtils.h>

namespace DAVA
{
namespace BrushRenderHelperDetails
{
rhi::HVertexBuffer InitPacket(rhi::Packet& packet)
{
    const int32 VERTEX_COUNT = 4;
    Array<Vector3, VERTEX_COUNT> quad =
    {
      Vector3(-1.0f, -1.0f, 0.0f),
      Vector3(-1.0f, +1.0f, 0.0f),
      Vector3(+1.0f, -1.0f, 0.0f),
      Vector3(+1.0f, +1.0f, 0.0f),
    };

    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(Vector3) * VERTEX_COUNT;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    rhi::HVertexBuffer vertexBuffer = rhi::CreateVertexBuffer(vDesc);

    packet.vertexStreamCount = 1;
    packet.vertexCount = 4;
    packet.primitiveType = rhi::PRIMITIVE_TRIANGLESTRIP;
    packet.primitiveCount = 2;
    packet.indexBuffer = rhi::HIndexBuffer();

    rhi::VertexLayout blitQuadLayout;
    blitQuadLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    packet.vertexLayoutUID = rhi::VertexLayout::UniqueId(blitQuadLayout);
    packet.vertexStream[0] = vertexBuffer;
    packet.vertexCount = VERTEX_COUNT;

    return vertexBuffer;
}

const int32 uvPickBlitPriorityOffset = 20;
const int32 applyBrushPriorityOffset = 19;
} // namespace name

UVPickTextureCopier::UVPickTextureCopier()
{
    vertexBuffer = BrushRenderHelperDetails::InitPacket(blitPacket);

    rhi::SamplerState::Descriptor samplerDescr;
    samplerDescr.fragmentSampler[0] = Renderer::GetRuntimeTextures().GetRuntimeTextureSamplerState(RuntimeTextures::TEXTURE_UVPICKING);
    samplerDescr.fragmentSamplerCount = 1;
    samplerDescr.vertexSamplerCount = 0;
    samplerState = rhi::AcquireSamplerState(samplerDescr);

    rhi::HTexture uvPickTexture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_UVPICKING);
    RhiUtils::VertexTextureSet vTexSet = {};
    RhiUtils::FragmentTextureSet fTexSet = { uvPickTexture };
    textureSet = RhiUtils::TextureSet(vTexSet, fTexSet);

    blitMaterial.Set(new NMaterial());
    blitMaterial->SetFXName(FastName(NMaterialName::TEXTURE_BLIT));

    FastName remapFlag("REMAP_TEX_COORD");
    blitMaterial->AddFlag(remapFlag, 1);

    FastName srcRectProperty("remapRect");
    Vector4 remapRect(0.0f, 1.0f, 0.0f, 1.0f);
    blitMaterial->AddProperty(srcRectProperty, remapRect.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
}

UVPickTextureCopier::~UVPickTextureCopier()
{
    rhi::DeleteVertexBuffer(vertexBuffer);
    rhi::ReleaseSamplerState(samplerState);
    rhi::ReleaseTextureSet(textureSet);
    blitMaterial.Set(nullptr);
}

void UVPickTextureCopier::BlitTextureRect(const Rect& srcRect, Asset<Texture> targetTexture, int32 basePriority)
{
    using namespace BrushRenderHelperDetails;

    rhi::RenderPassConfig passConfig;
    passConfig.name = "UVPickTexturesCopyPass";
    passConfig.priority = GetPriority(basePriority);
    passConfig.colorBuffer[0].texture = targetTexture->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = targetTexture->width;
    passConfig.viewport.height = targetTexture->height;

    Size2i srcSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_UVPICKING);

    float32 srcWidth = static_cast<float32>(srcSize.dx);
    float32 srcHeight = static_cast<float32>(srcSize.dy);

    Vector4 remapRect(srcRect.x / srcWidth, (srcRect.x + srcRect.dx) / srcWidth,
                      srcRect.y / srcWidth, (srcRect.y + srcRect.dy) / srcWidth);

    FastName srcRectProperty("remapRect");
    blitMaterial->SetPropertyValue(srcRectProperty, remapRect.data);

    bool prebuilded = blitMaterial->PreBuildMaterial(PASS_FORWARD);
    if (prebuilded == false)
    {
        DVASSERT(false);
        return;
    }

    blitMaterial->BindParams(blitPacket);
    blitPacket.textureSet = textureSet;
    blitPacket.samplerState = samplerState;

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    rhi::AddPackets(packetList, &blitPacket, 1);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

int32 UVPickTextureCopier::GetPriority(int32 basePriority) const
{
    return basePriority + PRIORITY_SERVICE_3D + BrushRenderHelperDetails::uvPickBlitPriorityOffset;
}

TextureBlitter::TextureBlitter()
{
    using namespace BrushRenderHelperDetails;
    vertexBuffer = InitPacket(blitPacket);

    blitMaterial.Set(new NMaterial());
    blitMaterial->SetFXName(FastName(NMaterialName::TEXTURE_BLIT));
    blitMaterial->AddFlag(FastName("TEXTURE_COUNT"), 1);
}

TextureBlitter::~TextureBlitter()
{
    rhi::DeleteVertexBuffer(vertexBuffer);
    blitMaterial.Set(nullptr);
}

void TextureBlitter::BlitTexture(const TargetInfo& renderTerget, RefPtr<NMaterial> material, int32 priority)
{
    rhi::RenderPassConfig passConfig;
    passConfig.name = "BlitTexturesPass";
    passConfig.priority = priority;
    passConfig.colorBuffer[0].texture = renderTerget.renderTarget->handle;
    passConfig.colorBuffer[0].textureLevel = renderTerget.textureLevel;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = renderTerget.renderTarget->width >> renderTerget.textureLevel;
    passConfig.viewport.height = renderTerget.renderTarget->height >> renderTerget.textureLevel;

    material->BindParams(blitPacket);

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    rhi::AddPackets(packetList, &blitPacket, 1);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}

BrushApplyHelper::BrushApplyHelper()
{
    using namespace BrushRenderHelperDetails;
    vertexBuffer = InitPacket(packet);
    packet.options |= rhi::Packet::OPT_OVERRIDE_SCISSOR;
}

BrushApplyHelper::~BrushApplyHelper()
{
    rhi::DeleteVertexBuffer(vertexBuffer);
}

bool BrushApplyHelper::ApplyBrush(const BaseTextureRenderLandscapeTool::BrushPhaseDescriptor& brush, const Rect& rect, int32 basePriority)
{
    using namespace DAVA;
    using namespace BrushRenderHelperDetails;
    if (brush.phaseMaterial->PreBuildMaterial(brush.passName) == false)
    {
        return false;
    }

    rhi::RenderPassConfig passConfig;
    passConfig.name = "ApplyBrushTexturesPass";
    passConfig.priority = GetPriority(basePriority);
    passConfig.colorBuffer[0].texture = brush.renderTarget->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].textureLevel = brush.renderTargetLevel;
    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = brush.renderTarget->width >> brush.renderTargetLevel;
    passConfig.viewport.height = brush.renderTarget->height >> brush.renderTargetLevel;

    brush.phaseMaterial->BindParams(packet);

    int16 scissorX = static_cast<int16>(std::floor(rect.x * passConfig.viewport.width));
    int16 scissorY = static_cast<int16>(std::floor(rect.y * passConfig.viewport.height));
    int16 scissorDX = static_cast<int16>(std::ceil(rect.dx * passConfig.viewport.width));
    int16 scissorDY = static_cast<int16>(std::ceil(rect.dy * passConfig.viewport.height));

    packet.scissorRect.x = Clamp<int16>(scissorX, 0, passConfig.viewport.width - 1);
    packet.scissorRect.y = Clamp<int16>(passConfig.viewport.height - scissorY - scissorDY, 0, passConfig.viewport.height);
    packet.scissorRect.width = scissorDX;
    packet.scissorRect.height = scissorDY;

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    rhi::AddPackets(packetList, &packet, 1);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);

    return true;
}

int32 BrushApplyHelper::GetPriority(int32 basePriority) const
{
    return basePriority + PRIORITY_SERVICE_3D + BrushRenderHelperDetails::applyBrushPriorityOffset;
}
} // namespace DAVA
