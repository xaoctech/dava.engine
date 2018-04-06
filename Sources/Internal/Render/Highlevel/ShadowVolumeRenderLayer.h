#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/UniqueStateSet.h"

namespace DAVA
{
class Camera;
class ShadowVolumeRenderLayer : public RenderLayer
{
public:
    ShadowVolumeRenderLayer(eRenderLayerID id, uint32 sortingFlags);
    virtual ~ShadowVolumeRenderLayer() override;

    void Draw(Camera* camera, const RenderBatchArray& renderBatchArray, rhi::HPacketList packetList, uint32 flags) override;

private:
    void PrepareRenderData();
    void Restore();

    NMaterial* shadowRectMaterial = nullptr;
    rhi::Packet shadowRectPacket;
    rhi::HVertexBuffer quadBuffer;
};

} // ns
