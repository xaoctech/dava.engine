#pragma once
#include "Render/Highlevel/LandscapeLayerRenderer.h"

#include "Render/RHI/rhi_Public.h"
#include "Render/Highlevel/Camera.h"
#include "Base/RefPtr.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/LandscapePageRenderer.h"

class Camera;

namespace DAVA
{
class VTDecalManager;
class DecalRenderObject;
class VTDecalPageRenderer : public LandscapePageRenderer
{
public:
    struct VTDecalVertex
    {
        Vector2 position;
        Vector3 uv; //uv, val
        Vector4 tangents; //t, b
    };

    VTDecalPageRenderer(bool useFetch);
    bool RenderPage(const PageRenderParams& params) override; //return false is nothing was rendered
    void SetVTDecalManager(VTDecalManager* manager);

private:
    void RenderDecal(DecalRenderObject* decal);
    void RenderSpline(DecalRenderObject* decal);
    void BlitSource(const PageRenderParams& params);

private:
    bool useFetch = false;
    rhi::Packet vtDecalsPacket, blitPacket;
    float32 vtPageInfo[4];
    float32 vtPos[2];
    float32 vtBasis[4];
    NMaterial* blitMaterial = nullptr;
    VTDecalManager* vtDecalManager = nullptr;
    Vector<DecalRenderObject*> clipResult;

    rhi::HPacketList packetList;
};
}
