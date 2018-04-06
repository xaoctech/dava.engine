#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/RHI/rhi_Public.h"

namespace DAVA
{
class Camera;
class RenderLayer : public InspBase
{
public:
    static eRenderLayerID GetLayerIDByName(const FastName& name);
    static const FastName& GetLayerNameByID(eRenderLayerID layer);

    // LAYERS SORTING FLAGS
    static const uint32 LAYER_SORTING_FLAGS_OPAQUE;
    static const uint32 LAYER_SORTING_FLAGS_AFTER_OPAQUE;
    static const uint32 LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER;
    static const uint32 LAYER_SORTING_FLAGS_WATER;
    static const uint32 LAYER_SORTING_FLAGS_TRANSLUCENT;
    static const uint32 LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT;
    static const uint32 LAYER_SORTING_FLAGS_SHADOW_VOLUME;
    static const uint32 LAYER_SORTING_FLAGS_VEGETATION;
    static const uint32 LAYER_SORTING_FLAGS_DEBUG_DRAW;

    RenderLayer(eRenderLayerID id, uint32 sortingFlags);
    virtual ~RenderLayer();

    inline eRenderLayerID GetRenderLayerID() const;
    inline uint32 GetSortingFlags() const;

    virtual void Draw(Camera* camera, const RenderBatchArray& batchArray, rhi::HPacketList packetList, uint32 flags);

    void SetViewportOverride(bool enable, const rhi::Viewport& vp);

    static uint32 MakeLayerMask(std::initializer_list<eRenderLayerID> layers);

protected:
    eRenderLayerID layerID;
    uint32 sortFlags;
    rhi::Viewport overridenViewport;
    bool overrideViewport = false;
};

inline eRenderLayerID RenderLayer::GetRenderLayerID() const
{
    return layerID;
}

inline uint32 RenderLayer::GetSortingFlags() const
{
    return sortFlags;
}
inline void RenderLayer::SetViewportOverride(bool enable, const rhi::Viewport& vp)
{
    overrideViewport = enable;
    overridenViewport = vp;
}

} // ns
