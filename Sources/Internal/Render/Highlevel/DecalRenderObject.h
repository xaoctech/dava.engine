#pragma once

#include "Render/Highlevel/RenderObject.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class DecalRenderObject : public RenderObject
{
public:
    struct SplineRenderData
    {
        struct SegmentData
        {
            AABBox2 bbox;
            AABBox2 worldBbox;
            int32 vertexCount;
            int32 indexCount;
        };
        Vector<SegmentData> segmentData;

        rhi::HVertexBuffer vertexBuffer;
        rhi::HIndexBuffer indexBuffer;
        AABBox2 resBox;

        uint32 sliceCount;

        //GFX_COMPLETE VTDecalManager may fill this data each clip. Still thinking would it be ok, or we can skip searching this once any segment box is visible and draw whole spline instead
        int32 clippedBaseVertex = 0;
        int32 clippedVertexCount = 0;
        int32 clippedBaseIndex = 0;
        int32 clippedIndexCount = 0;

        ~SplineRenderData();
    };
    enum DecalDomain
    {
        DOMAIN_GBUFFER,
        DOMAIN_VT
    };

    DecalRenderObject();
    ~DecalRenderObject();

    void RecalcBoundingBox() override;

    void RecalculateWorldBoundingBox() override;

    void SetDecalSize(const Vector3& size);
    const Vector3& GetDecalSize() const;

    void SetSplineData(SplineRenderData* splineData);
    SplineRenderData* GetSplineData() const;
    void SplineDataUpdated();

    uint64 GetSortingKey() const;
    uint32 GetGbufferMask() const;

    NMaterial* GetMaterial() const;
    void SetMaterial(NMaterial* material);

    void SetDomain(DecalDomain domain);
    DecalDomain GetDomain() const;

    void SetWireframe(bool wireframe);
    bool GetWireframe() const;
    /**
    * should be within range 0..15
    */
    void SetSortingOffset(uint32 offset);

private:
    void RecalcSortingKey();

private:
    RefPtr<NMaterial> material;
    Vector3 size = Vector3(1.0, 1.0, 1.0);

    uint32 sortingOffset = 7;
    uint64 sortingKey = uint64(-1); //GFX_COMPLETE - rebuild sorting key
    uint32 gbufferMask = 0;
    DecalDomain domain = DOMAIN_GBUFFER;

    SplineRenderData* splineRenderData = nullptr;
    bool wireframeMode = false;

    DAVA_VIRTUAL_REFLECTION(DecalRenderObject, RenderObject);
};

inline uint64 DecalRenderObject::GetSortingKey() const
{
    return sortingKey;
}

inline uint32 DecalRenderObject::GetGbufferMask() const
{
    return gbufferMask;
}

inline const Vector3& DecalRenderObject::GetDecalSize() const
{
    return size;
}

inline void DecalRenderObject::SetMaterial(NMaterial* material_)
{
    material = material_;
    RecalcSortingKey();
}

inline NMaterial* DecalRenderObject::GetMaterial() const
{
    return material.Get();
}

inline void DecalRenderObject::SetSortingOffset(uint32 offset)
{
    DVASSERT(offset <= 15);
    sortingOffset = offset;
    RecalcSortingKey();
}

inline void DecalRenderObject::SetDomain(DecalRenderObject::DecalDomain domain_)
{
    domain = domain_;
}

inline DecalRenderObject::DecalDomain DecalRenderObject::GetDomain() const
{
    return domain;
}

inline void DecalRenderObject::RecalcSortingKey()
{
    const uint64 offsetMask = 0x0f;
    const uint64 pointerMask = 0x0fffffffffffffff;
    const uint64 offsetShift = 60;
    uint64 pointerKey = uint64(material.Get()) & pointerMask;
    uint64 offsetKey = uint64(sortingOffset & offsetMask) << offsetShift;
    sortingKey = pointerKey + offsetKey;
}

inline void DecalRenderObject::SetWireframe(bool wireframe)
{
    wireframeMode = wireframe;
}
inline bool DecalRenderObject::GetWireframe() const
{
    return wireframeMode;
}
}
