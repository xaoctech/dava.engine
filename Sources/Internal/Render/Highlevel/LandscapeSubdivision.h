#pragma once

#include <Base/Any.h>
#include "Base/BaseTypes.h"
#include "Math/AABBox3.h"
#include "Base/AllocatorFactory.h"
#include "Reflection/Reflection.h"
#include "Base/Introspection.h"
#include "Base/IntrospectionBase.h"
#include "MemoryManager/MemoryProfiler.h"

namespace DAVA
{
class Frustum;
class Heightmap;
class Camera;

class LandscapeSubdivision : public InspBase
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_LANDSCAPE)

public:
    LandscapeSubdivision();
    ~LandscapeSubdivision();

    struct SubdivisionPatch
    {
        IMPLEMENT_POOL_ALLOCATOR(SubdivisionPatch, 256);

        static uint32 count;

        SubdivisionPatch()
        {
            Memset(children, 0, sizeof(children));
        };
        ~SubdivisionPatch();

        /////////////////////////////////////////////////

        SubdivisionPatch* parent = nullptr;
        SubdivisionPatch* children[4];

        AABBox3 bbox;
        float32 patchRadius = 0.f;

        uint32 level = 0;
        uint32 x = 0;
        uint32 y = 0;

        float32 morphCoeff = 0.f;

        Vector3 positionOfMaxError;
        float32 maxError = 0.f;
        float32 radiusError = 0.f;
        float32 heightError = 0.f;

        uint8 startClipPlane = 0;

        bool isTerminated = false;
    };

    struct SubdivisionMetrics : public InspBase
    {
        float32 normalFov = 70.f;
        float32 zoomFov = 6.5f;

        float32 normalMaxHeightError = 0.02f;
        float32 normalMaxPatchRadiusError = 0.33f;
        float32 normalMaxAbsoluteHeightError = 12.f;

        float32 zoomMaxHeightError = 0.04f;
        float32 zoomMaxPatchRadiusError = 0.66f;
        float32 zoomMaxAbsoluteHeightError = 12.f;

        bool operator==(const SubdivisionMetrics& other) const;

        DAVA_VIRTUAL_REFLECTION(SubdivisionMetrics, InspBase);
    };

    void BuildSubdivision(Heightmap* heightmap, const AABBox3& bbox, uint32 patchSizeQuads);
    void UpdateHeightChainData(const Rect2i& heighmapRect);
    const SubdivisionPatch* PrepareSubdivision(Camera* camera, const Matrix4* worldTransform, uint32* terminatedCount = nullptr);
    void ReleaseInternalData();

    const SubdivisionMetrics& GetMetrics() const;
    void SetMetrics(const SubdivisionMetrics& metrics);

    void SetMinSubdivisionLevel(uint32 level);
    void SetMaxSubdivisionLevel(uint32 level);

    void SetPatchBBoxGap(float32 min, float32 max);

    void SetForceMinSubdivision(uint32 level);

    static uint32 GetLevelSize(uint32 level);
    static uint32 GetLevelArrayOffset(uint32 level);

    float32 GetSubdivisionDistance(uint32 level) const;
    AABBox3 GetPatchAABBox(uint32 level, uint32 x, uint32 y) const;
    const AABBox3& GetSubdivisionAABBox() const;

private:
    struct HeightChainElement
    {
        uint16 max;
        uint16 min;
    };

    struct HeightErrorChainElement
    {
        Vector3 positionOfMaxError;
        float32 maxError;
    };

    void UpdateHeightChainData(uint32 level, uint32 x, uint32 y, HeightChainElement* parentChainElement, HeightErrorChainElement* parentErrorChainElement, const Rect2i& updateRect);
    void InvalidateSubdivision(SubdivisionPatch* subdividePatch);
    void SubdividePatch(SubdivisionPatch*& subdividePatch, SubdivisionPatch* parent, uint32 level, uint32 x, uint32 y, uint8 clippingFlags);

    uint32 GetChainIndex(uint32 level, uint32 x, uint32 y) const;

    Vector<HeightChainElement> heightChain;
    Vector<HeightErrorChainElement> heightErrorChain;

    uint32 heightChainLevelCount = 0;
    uint32 heightErrorChainLevelCount = 0;

    uint32 minSubdivideLevel = 0;
    uint32 maxSubdivideLevel = 0;
    uint32 forceMinSubdiveLevel = 0;

    Map<Camera*, SubdivisionPatch*> subdivisionRoots;
    uint32 currentTerminatedPatchCount = 0;

    uint32 patchSizeQuads = 8;

    SubdivisionMetrics metrics;

    float32 maxHeightError = 0.f;
    float32 maxPatchRadiusError = 0.f;
    float32 maxAbsoluteHeightError = 0.f;

    Vector3 cameraPos;
    float32 tanFovY = 0.f;

    float32 patchBBoxGapMin = 0.f;
    float32 patchBBoxGapMax = 0.f;

    Frustum* frustum = nullptr;
    Heightmap* heightmap = nullptr;

    AABBox3 subdivisionBBox;

    friend class LandscapeSystem;

    DAVA_VIRTUAL_REFLECTION(LandscapeSubdivision, InspBase);
};

inline uint32 LandscapeSubdivision::GetLevelSize(uint32 level)
{
    return (1 << level);
}

inline uint32 LandscapeSubdivision::GetLevelArrayOffset(uint32 level)
{
    uint32 offset = 0;
    for (uint32 p = 0; p < (level << 1); p += 2)
        offset += (1 << p);

    return offset;
}

inline uint32 LandscapeSubdivision::GetChainIndex(uint32 level, uint32 x, uint32 y) const
{
    DVASSERT(x < GetLevelSize(level) && y < GetLevelSize(level));
    return GetLevelArrayOffset(level) + (y << level) + x;
}

inline void LandscapeSubdivision::SetForceMinSubdivision(uint32 level)
{
    forceMinSubdiveLevel = level;
}

inline const LandscapeSubdivision::SubdivisionMetrics& LandscapeSubdivision::GetMetrics() const
{
    return metrics;
}

inline void LandscapeSubdivision::SetMetrics(const SubdivisionMetrics& _metrics)
{
    metrics = _metrics;
}

inline const AABBox3& LandscapeSubdivision::GetSubdivisionAABBox() const
{
    return subdivisionBBox;
}

template <>
bool AnyCompare<LandscapeSubdivision::SubdivisionMetrics>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<LandscapeSubdivision::SubdivisionMetrics>;
}
