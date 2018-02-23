#pragma once

#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
class NMaterial;
class QuadRenderer
{
public:
    QuadRenderer();
    ~QuadRenderer();

    struct Options
    {
        NMaterial* material = 0; //material to draw
        rhi::Handle dstTexture = 0; //destination texture
        Vector2 dstTexSize; //destination texture size
        Rect2f dstRect; //rect in destination texture where to draw
        rhi::LoadAction loadAction = rhi::LoadAction::LOADACTION_NONE; //destination load action

        rhi::HTexture srcTexture; //texture to draw
        Vector2 srcTexSize; //source texture size
        Rect2f srcRect; //rect in source texture to draw
        rhi::HTextureSet textureSet; //optional texture set, replaces srcTexture if present
        rhi::HSamplerState samplerState; //optional sampler state, replaces default if present

        uint32 textureLevel = 0; //texture mip
        rhi::TextureFace textureFace = rhi::TEXTURE_FACE_NONE; //optional texture face
        int32 renderPassPriority = 3;
        const char* renderPassName = DEFAULT_DEBUG_RENDERPASS_NAME;
    };

    void Render(const QuadRenderer::Options& options);
    void RenderClear(const QuadRenderer::Options& options);
    void Render(const char* tag, NMaterial* withMaterial, rhi::Viewport viewport, rhi::Handle destination, rhi::TextureFace textureFace,
                uint32 textureLevel, rhi::LoadAction loadAction = rhi::LoadAction::LOADACTION_NONE, int32 priorityOffset = 0);
    void RenderToPacketList(rhi::HPacketList pl, NMaterial* withMaterial);

    static const char* DEFAULT_DEBUG_RENDERPASS_NAME;

public:
    rhi::Packet rectPacket;

private:
    rhi::HVertexBuffer quadBuffer;
    rhi::RenderPassConfig passConfig;
    rhi::HDepthStencilState depthStencilState;
};
}
