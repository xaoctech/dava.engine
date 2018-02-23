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
    void BlitSourceWithTerrainTargets(const PageRenderParams& params);
    void InitTerrainBlendTargets(const PageRenderParams& params);

private:
    bool useFetch = false;
    rhi::Packet vtDecalsPacket, blitPacket, blendTerrainPacket;
    float32 vtPageInfo[4];
    float32 vtPos[2];
    float32 vtBasis[4];
    RefPtr<NMaterial> blitMaterial;
    VTDecalManager* vtDecalManager = nullptr;
    Vector<DecalRenderObject*> clipResult;

    Vector<RefPtr<Texture>> blendTargetsTerrain;
    uint32 blendTargetTerrainSize = 0;
    RefPtr<NMaterial> blendTerrainMaterial;

    rhi::HPacketList packetList;
};
}
