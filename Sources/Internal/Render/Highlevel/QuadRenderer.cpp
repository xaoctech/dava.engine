#include "Render/Highlevel/QuadRenderer.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/DynamicBindings.h"
#include "Debug/ProfilerGPU.h"
#include "Render/RhiUtils.h"

namespace DAVA
{
const char* QuadRenderer::DEFAULT_DEBUG_RENDERPASS_NAME = "???????";

QuadRenderer::QuadRenderer()
{
    const static uint32 VERTEX_COUNT = 3;

    std::array<Vector3, VERTEX_COUNT> quad =
    {
      Vector3(-1.0f, -1.0f, +1.0f),
      Vector3(-1.0f, +3.0f, +1.0f),
      Vector3(+3.0f, -1.0f, +1.0f),
    };

    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(Vector3) * VERTEX_COUNT;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    rhi::VertexLayout vxLayout;
    vxLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    rectPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(vxLayout);

    rectPacket.vertexStreamCount = 1;
    rectPacket.vertexStream[0] = quadBuffer;
    rectPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    rectPacket.primitiveCount = VERTEX_COUNT / 3;

    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 1.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 1.0f;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    rhi::DepthStencilState::Descriptor ds;
    ds.depthTestEnabled = false;
    ds.depthWriteEnabled = false;
    ds.depthFunc = rhi::CMP_ALWAYS;
    depthStencilState = rhi::AcquireDepthStencilState(ds);
}

QuadRenderer::~QuadRenderer()
{
    rhi::DeleteVertexBuffer(quadBuffer);
}

void QuadRenderer::RenderClear(const QuadRenderer::Options& options)
{
    DVASSERT(options.srcRect.dx != 0 && options.srcRect.dy != 0);
    DVASSERT(options.dstRect.dx != 0 && options.dstRect.dy != 0);
    DVASSERT(options.srcTexSize.x != 0 && options.srcTexSize.y != 0);
    DVASSERT(options.dstTexSize.x != 0 && options.dstTexSize.y != 0);
    passConfig.viewport = rhi::Viewport(uint32(options.dstRect.x), uint32(options.dstRect.y), uint32(options.dstRect.dx), uint32(options.dstRect.dy));
    passConfig.colorBuffer[0].loadAction = options.loadAction;
    passConfig.colorBuffer[0].texture = options.dstTexture;
    passConfig.colorBuffer[0].textureFace = options.textureFace;
    passConfig.colorBuffer[0].textureLevel = options.textureLevel;
    passConfig.priority = PRIORITY_MAIN_3D + options.renderPassPriority;

    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, options.renderPassName);
    rhi::HPacketList packetList;
    rhi::HRenderPass renderPass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    if (renderPass != rhi::InvalidHandle)
    {
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);
        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(renderPass);
    }
}

void QuadRenderer::Render(const QuadRenderer::Options& options)
{
    DVASSERT(options.srcRect.dx != 0 && options.srcRect.dy != 0);
    DVASSERT(options.dstRect.dx != 0 && options.dstRect.dy != 0);
    DVASSERT(options.srcTexSize.x != 0 && options.srcTexSize.y != 0);
    DVASSERT(options.dstTexSize.x != 0 && options.dstTexSize.y != 0);
    passConfig.viewport = rhi::Viewport(uint32(options.dstRect.x), uint32(options.dstRect.y), uint32(options.dstRect.dx), uint32(options.dstRect.dy));
    passConfig.colorBuffer[0].loadAction = options.loadAction;
    passConfig.colorBuffer[0].texture = options.dstTexture;
    passConfig.colorBuffer[0].textureFace = options.textureFace;
    passConfig.colorBuffer[0].textureLevel = options.textureLevel;
    passConfig.priority = PRIORITY_MAIN_3D + options.renderPassPriority;
    passConfig.name = options.renderPassName;

    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, options.renderPassName);
    rhi::HPacketList packetList;
    rhi::HRenderPass renderPass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    if (renderPass != rhi::InvalidHandle)
    {
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);

        if (options.material->HasLocalProperty(FastName("srcRectOffset")))
            options.material->SetPropertyValue(FastName("srcRectOffset"), options.srcRect.GetPosition().data);

        if (options.material->HasLocalProperty(FastName("srcRectSize")))
            options.material->SetPropertyValue(FastName("srcRectSize"), options.srcRect.GetSize().data);

        if (options.material->HasLocalProperty(FastName("srcTexSize")))
            options.material->SetPropertyValue(FastName("srcTexSize"), options.srcTexSize.data);

        if (options.material->HasLocalProperty(FastName("destTexSize")))
            options.material->SetPropertyValue(FastName("destTexSize"), options.dstTexSize.data);

        if (options.material->PreBuildMaterial(PASS_FORWARD))
        {
            options.material->BindParams(rectPacket);

            if (options.textureSet.IsValid())
            {
                rectPacket.textureSet = options.textureSet;
            }
            else
            {
                rectPacket.textureSet = RhiUtils::FragmentTextureSet{ options.srcTexture };
            }

            if (options.samplerState.IsValid())
            {
                rectPacket.samplerState = options.samplerState;
            }

            rectPacket.depthStencilState = depthStencilState;

            rhi::AddPacket(packetList, rectPacket);
        }

        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(renderPass);
    }
}

void QuadRenderer::Render(const char* tag, NMaterial* withMaterial, rhi::Viewport viewport, rhi::Handle destination, rhi::TextureFace textureFace,
                          uint32 textureLevel, rhi::LoadAction loadAction, int32 priorityOffset)
{
    passConfig.viewport = viewport;

    passConfig.colorBuffer[0].texture = destination;
    passConfig.colorBuffer[0].textureFace = textureFace;
    passConfig.colorBuffer[0].textureLevel = textureLevel;
    passConfig.colorBuffer[0].loadAction = loadAction;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.priority = PRIORITY_SERVICE_3D + priorityOffset; // GFX_COMPLETE think about priorities

    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, tag);
    rhi::HPacketList packetList;
    rhi::HRenderPass renderPass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
    if (renderPass != rhi::InvalidHandle)
    {
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);
        RenderToPacketList(packetList, withMaterial);
        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(renderPass);
    }
}

void QuadRenderer::RenderToPacketList(rhi::HPacketList pl, NMaterial* withMaterial)
{
    withMaterial->BindParams(rectPacket);
    rectPacket.depthStencilState = depthStencilState;
    rhi::AddPacket(pl, rectPacket);
}
}
