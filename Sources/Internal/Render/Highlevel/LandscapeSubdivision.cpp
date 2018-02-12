#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/RHI/rhi_Public.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeSubdivision::SubdivisionMetrics)
{
    ReflectionRegistrator<SubdivisionMetrics>::Begin()
    .Field("normalMaxHeightError", &SubdivisionMetrics::normalMaxHeightError)[M::DisplayName("Maximum height error"), M::Group("Normal")]
    .Field("normalMaxPatchRadiusError", &SubdivisionMetrics::normalMaxPatchRadiusError)[M::DisplayName("Maximum patch radius error"), M::Group("Normal")]
    .Field("normalMaxAbsoluteHeightError", &SubdivisionMetrics::normalMaxAbsoluteHeightError)[M::DisplayName("Maximum absolute height error"), M::Group("Normal")]
    .Field("zoomMaxHeightError", &SubdivisionMetrics::zoomMaxHeightError)[M::DisplayName("Maximum height error"), M::Group("Zoom")]
    .Field("zoomMaxPatchRadiusError", &SubdivisionMetrics::zoomMaxPatchRadiusError)[M::DisplayName("Maximum patch radius error"), M::Group("Zoom")]
    .Field("zoomMaxAbsoluteHeightError", &SubdivisionMetrics::zoomMaxAbsoluteHeightError)[M::DisplayName("Maximum absolute height error"), M::Group("Zoom")]
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeSubdivision)
{
    ReflectionRegistrator<LandscapeSubdivision>::Begin()
    .Field("metrics", &LandscapeSubdivision::metrics)[M::DisplayName("Metrics")]
    .End();
}

template <>
bool AnyCompare<LandscapeSubdivision::SubdivisionMetrics>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<LandscapeSubdivision::SubdivisionMetrics>() == v2.Get<LandscapeSubdivision::SubdivisionMetrics>();
}

bool LandscapeSubdivision::SubdivisionMetrics::operator==(const SubdivisionMetrics& other) const
{
    return normalFov == other.normalFov &&
    zoomFov == other.zoomFov &&
    normalMaxHeightError == other.normalMaxHeightError &&
    normalMaxPatchRadiusError == other.normalMaxPatchRadiusError &&
    normalMaxAbsoluteHeightError == other.normalMaxAbsoluteHeightError &&
    zoomMaxHeightError == other.zoomMaxHeightError &&
    zoomMaxPatchRadiusError == other.zoomMaxPatchRadiusError &&
    zoomMaxAbsoluteHeightError == other.zoomMaxAbsoluteHeightError;
}

LandscapeSubdivision::SubdivisionPatch::~SubdivisionPatch()
{
    for (SubdivisionPatch*& c : children)
        SafeDelete(c);
}

LandscapeSubdivision::LandscapeSubdivision()
{
    frustum = new Frustum();
}

LandscapeSubdivision::~LandscapeSubdivision()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeRelease(frustum);
    SafeRelease(heightmap);
}

void LandscapeSubdivision::ReleaseInternalData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    for (std::pair<Camera*, SubdivisionPatch*> root : subdivisionRoots)
        SafeDelete(root.second);

    subdivisionRoots.clear();
    heightChain.clear();

    SafeRelease(heightmap);
}

void LandscapeSubdivision::BuildSubdivision(Heightmap* _heightmap, const AABBox3& _bbox, uint32 _patchSizeQuads)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ReleaseInternalData();

    DVASSERT(_heightmap);

    heightmap = SafeRetain(_heightmap);
    subdivisionBBox = _bbox;
    patchSizeQuads = _patchSizeQuads;

    heightChainLevelCount = FastLog2(heightmap->Size()) + 1;
    heightErrorChainLevelCount = FastLog2(heightmap->Size() / patchSizeQuads) + 1;

    heightChain.resize(GetLevelArrayOffset(heightChainLevelCount));
    heightErrorChain.resize(GetLevelArrayOffset(heightErrorChainLevelCount));

    UpdateHeightChainData(Rect2i(0, 0, -1, -1));
}

void LandscapeSubdivision::SetMinSubdivisionLevel(uint32 level)
{
    minSubdivideLevel = level;
    maxSubdivideLevel = Max(minSubdivideLevel, maxSubdivideLevel);
}

void LandscapeSubdivision::SetMaxSubdivisionLevel(uint32 level)
{
    maxSubdivideLevel = level;
    minSubdivideLevel = Min(minSubdivideLevel, maxSubdivideLevel);
}

void LandscapeSubdivision::SetPatchBBoxGap(float32 min, float32 max)
{
    patchBBoxGapMin = min;
    patchBBoxGapMax = max;
}

void LandscapeSubdivision::UpdateHeightChainData(const Rect2i& heighmapRect)
{
    UpdateHeightChainData(0, 0, 0, nullptr, nullptr, heighmapRect);

    for (std::pair<Camera*, SubdivisionPatch*> root : subdivisionRoots)
        InvalidateSubdivision(root.second);
}

void LandscapeSubdivision::InvalidateSubdivision(SubdivisionPatch* patch)
{
    if (patch)
    {
        InvalidateSubdivision(patch->children[0]);
        InvalidateSubdivision(patch->children[1]);
        InvalidateSubdivision(patch->children[2]);
        InvalidateSubdivision(patch->children[3]);

        patch->bbox.Empty();
    }
}

void LandscapeSubdivision::UpdateHeightChainData(uint32 level, uint32 x, uint32 y, HeightChainElement* parentChainElement, HeightErrorChainElement* parentErrorChainElement, const Rect2i& updateRect)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if (level >= heightChainLevelCount)
        return;

    int32 hmSize = heightmap->Size();
    uint32 patchSize = hmSize >> level; //patch size in 'heightmap quads'
    DVASSERT(patchSize);

    uint32 xx = x * patchSize;
    uint32 yy = y * patchSize;

    if (updateRect.dx >= 0 && updateRect.dy >= 0 && !Rect2i(xx, yy, patchSize, patchSize).RectIntersects(updateRect))
        return;

    HeightChainElement& hce = heightChain[GetChainIndex(level, x, y)];
    hce.max = 0;
    hce.min = std::numeric_limits<uint16>::max();

    HeightErrorChainElement* hece = nullptr;
    if (level < heightErrorChainLevelCount)
    {
        hece = &heightErrorChain[GetChainIndex(level, x, y)];
        hece->maxError = 0.f;
        hece->positionOfMaxError = Vector3();
    }

    uint32 step = Max(patchSize / patchSizeQuads, 1u);
    for (uint32 y0 = yy; y0 < yy + patchSize; y0 += step)
    {
        for (uint32 x0 = xx; x0 < xx + patchSize; x0 += step)
        {
            //find min/max height of patch
            uint16 h0[4] = {
                heightmap->GetHeight(x0, y0),
                heightmap->GetHeightClamp(x0, y0 + step),
                heightmap->GetHeightClamp(x0 + step, y0),
                heightmap->GetHeightClamp(x0 + step, y0 + step)
            };

            hce.min = Min(hce.min, Min(Min(h0[0], h0[1]), Min(h0[2], h0[3])));
            hce.max = Max(hce.max, Max(Max(h0[0], h0[1]), Max(h0[2], h0[3])));

            //calculate height errors
            if (level < (heightErrorChainLevelCount - 1))
            {
                DVASSERT(step >= 2);

                uint32 x_ = x0 + (step >> 1);
                uint32 y_ = y0 + (step >> 1);
                uint32 x1 = x0 + step;
                uint32 y1 = y0 + step;

                //Patch corners points
                Vector3 p00 = heightmap->GetPoint(x0, y0, subdivisionBBox);
                Vector3 p01 = heightmap->GetPoint(x0, y1, subdivisionBBox);
                Vector3 p10 = heightmap->GetPoint(x1, y0, subdivisionBBox);
                Vector3 p11 = heightmap->GetPoint(x1, y1, subdivisionBBox);

                //Calculating max absolute height error between neighbour lods.
                //Choosing from five averaged heights per quad: four on middle of edges and one on diagonal
                // +---*---+
                // |     / |
                // |    /  |
                // *   *   *
                // |  /    |
                // | /     |
                // +---*---+

                //Accurate height values from next subdivide level (more detailed LOD)
                Vector3 p0[5] = {
                    heightmap->GetPoint(x_, y0, subdivisionBBox),
                    heightmap->GetPoint(x0, y_, subdivisionBBox),
                    heightmap->GetPoint(x_, y_, subdivisionBBox),
                    heightmap->GetPoint(x1, y_, subdivisionBBox),
                    heightmap->GetPoint(x_, y1, subdivisionBBox),
                };
                //Averaged height values from current level (less detailed LOD)
                float32 h1[5] = {
                    (p00.z + p10.z) / 2.f,
                    (p00.z + p01.z) / 2.f,
                    (p10.z + p01.z) / 2.f,
                    (p10.z + p11.z) / 2.f,
                    (p01.z + p11.z) / 2.f,
                };

                //Calculate max error for quad
                for (int32 i = 0; i < 5; ++i)
                {
                    float32 error = p0[i].z - h1[i];
                    if (Abs(hece->maxError) < Abs(error))
                    {
                        hece->maxError = error;
                        hece->positionOfMaxError = p0[i];
                    }
                }
            }
        }
    }

    uint32 x2 = x << 1;
    uint32 y2 = y << 1;

    //UpdateHeightChainData can modify maxError-info and height-info of parentPatch
    UpdateHeightChainData(level + 1, x2 + 0, y2 + 0, &hce, hece, updateRect);
    UpdateHeightChainData(level + 1, x2 + 1, y2 + 0, &hce, hece, updateRect);
    UpdateHeightChainData(level + 1, x2 + 0, y2 + 1, &hce, hece, updateRect);
    UpdateHeightChainData(level + 1, x2 + 1, y2 + 1, &hce, hece, updateRect);

    if (parentChainElement)
    {
        parentChainElement->max = Max(parentChainElement->max, hce.max);
        parentChainElement->min = Min(parentChainElement->min, hce.min);
    }

    if (hece && parentErrorChainElement)
    {
        if (Abs(parentErrorChainElement->maxError) < Abs(hece->maxError))
        {
            parentErrorChainElement->maxError = hece->maxError;
            parentErrorChainElement->positionOfMaxError = hece->positionOfMaxError;
        }
    }
}

const LandscapeSubdivision::SubdivisionPatch* LandscapeSubdivision::PrepareSubdivision(Camera* camera, const Matrix4* worldTransform, uint32* terminatedPatchCount)
{
    cameraPos = camera->GetPosition();

    frustum->Build((*worldTransform) * camera->GetViewProjMatrix(false, camera->GetReverseZEnabled()), rhi::DeviceCaps().isZeroBaseClipRange);

    float32 fovLerp = Clamp((camera->GetFOV() - metrics.zoomFov) / (metrics.normalFov - metrics.zoomFov), 0.f, 1.f);
    maxHeightError = metrics.zoomMaxHeightError + (metrics.normalMaxHeightError - metrics.zoomMaxHeightError) * fovLerp;
    maxPatchRadiusError = metrics.zoomMaxPatchRadiusError + (metrics.normalMaxPatchRadiusError - metrics.zoomMaxPatchRadiusError) * fovLerp;
    maxAbsoluteHeightError = metrics.zoomMaxAbsoluteHeightError + (metrics.normalMaxAbsoluteHeightError - metrics.zoomMaxAbsoluteHeightError) * fovLerp;

    //tanFovY used for calculate metrics projection on screen. Projection calculate as '1.0 / (distance * tan(fov / 2))'. See errors calculation in SubdividePatch()
    tanFovY = tanf(camera->GetFOV() * PI / 360.f) / camera->GetAspect();

    currentTerminatedPatchCount = 0;
    SubdivisionPatch*& subdivisionRoot = subdivisionRoots[camera];
    SubdividePatch(subdivisionRoot, nullptr, 0, 0, 0, 0x3f);

    if (terminatedPatchCount)
        *terminatedPatchCount = currentTerminatedPatchCount;

    return subdivisionRoot;
}

void LandscapeSubdivision::SubdividePatch(SubdivisionPatch*& subdivPatch, SubdivisionPatch* parent, uint32 level, uint32 x, uint32 y, uint8 clippingFlags)
{
    uint8 startClipPlane = subdivPatch ? subdivPatch->startClipPlane : 0;
    AABBox3 bbox = (subdivPatch && !subdivPatch->bbox.IsEmpty()) ? subdivPatch->bbox : GetPatchAABBox(level, x, y);

    //frustum-culling
    if (clippingFlags && (frustum->Classify(bbox, clippingFlags, startClipPlane) == Frustum::EFR_OUTSIDE))
    {
        SafeDelete(subdivPatch);
        return;
    }

    if (!subdivPatch)
    {
        subdivPatch = new SubdivisionPatch();
        subdivPatch->level = level;
        subdivPatch->x = x;
        subdivPatch->y = y;
        subdivPatch->bbox = bbox;
        subdivPatch->patchRadius = Distance(bbox.GetCenter(), bbox.max);
        subdivPatch->startClipPlane = startClipPlane;
        subdivPatch->parent = parent;
    }

    ////////////////////////////////////////////////////////////////////////////////////

    //Metrics errors we calculate as projection on screen
    //
    //                     /              |
    //                  /                 ^ - error in world-space
    //               /                    |
    //            /                       |
    //         /|                         |
    //      /   ^ - error in screen-space |
    //   /      |                         |
    //  0---------------------------------D-------- frustum axis
    //          ^                         ^
    //      near plane            error position plane
    // plane size 1.0 a-priory   plane size let it be 'H'
    //
    // H = D * tg(fov/2), were D - is distance to error position
    // To find error size on near plane we need just error size divide by 'H'
    // So, screen space error = error / (D * tg(fov/2))
    // tg(fov/2) calculating one per-frame, see 'tanFovY' in PrepareSubdivision()

    float32 patchDistance = Distance(cameraPos, bbox.GetCenter());
    float32 radiusError = subdivPatch->patchRadius / (patchDistance * tanFovY);

    float32 heightError = 0.f;
    float32 maxError = 0.f;
    if (level < heightErrorChainLevelCount)
    {
        const HeightErrorChainElement& hece = heightErrorChain[GetChainIndex(level, x, y)];
        maxError = hece.maxError;
        heightError = Abs(maxError) / (Distance(cameraPos, hece.positionOfMaxError) * tanFovY);

        subdivPatch->positionOfMaxError = hece.positionOfMaxError;
    }

    subdivPatch->radiusError = radiusError;
    subdivPatch->heightError = heightError;
    subdivPatch->maxError = maxError;

    //calculate morph value
    {
        float32 radiusError0 = parent ? parent->radiusError : maxPatchRadiusError;
        float32 heightError0 = parent ? parent->heightError : maxHeightError;

        float32 radiusError0Rel = Max(radiusError0, maxPatchRadiusError) / maxPatchRadiusError;
        float32 radiusErrorRel = Min(radiusError, maxPatchRadiusError) / maxPatchRadiusError;

        float32 heightError0Rel = Max(heightError0, maxHeightError) / maxHeightError;
        float32 heightErrorRel = Min(heightError, maxHeightError) / maxHeightError;

        float32 error0Delta = Max(radiusError0Rel, heightError0Rel) - 1.f;
        float32 errorDelta = 1.f - Max(radiusErrorRel, heightErrorRel);

        subdivPatch->morphCoeff = 1.f - errorDelta / (error0Delta + errorDelta);
    }

    if ((level < maxSubdivideLevel) && ((maxPatchRadiusError <= radiusError) || (maxHeightError <= heightError) || (maxAbsoluteHeightError < Abs(maxError)) || (minSubdivideLevel > level) || (forceMinSubdiveLevel > level)))
    {
        subdivPatch->isTerminated = false;

        uint32 x2 = x << 1;
        uint32 y2 = y << 1;

        SubdividePatch(subdivPatch->children[0], subdivPatch, level + 1, x2 + 0, y2 + 0, clippingFlags);
        SubdividePatch(subdivPatch->children[1], subdivPatch, level + 1, x2 + 1, y2 + 0, clippingFlags);
        SubdividePatch(subdivPatch->children[2], subdivPatch, level + 1, x2 + 0, y2 + 1, clippingFlags);
        SubdividePatch(subdivPatch->children[3], subdivPatch, level + 1, x2 + 1, y2 + 1, clippingFlags);
    }
    else
    {
        subdivPatch->isTerminated = true;
        ++currentTerminatedPatchCount;

        SafeDelete(subdivPatch->children[0]);
        SafeDelete(subdivPatch->children[1]);
        SafeDelete(subdivPatch->children[2]);
        SafeDelete(subdivPatch->children[3]);
    }
}

inline float32 InterpolateHeight(float32 x, float32 y, float32 h00, float32 h10, float32 h01, float32 h11)
{
    return h00 + x * (h10 - h00) + y * (h01 - h00) + x * y * (h00 + h11 - h10 - h01);
}

AABBox3 LandscapeSubdivision::GetPatchAABBox(uint32 level, uint32 x, uint32 y) const
{
    AABBox3 bbox;

    float32 levelSizef = float32(GetLevelSize(level));
    Vector3 bboxSize = subdivisionBBox.GetSize();

    bbox.min.x = (subdivisionBBox.min.x + (x / levelSizef) * bboxSize.x);
    bbox.max.x = (subdivisionBBox.min.x + ((x + 1) / levelSizef) * bboxSize.x);

    bbox.min.y = (subdivisionBBox.min.y + (y / levelSizef) * bboxSize.y);
    bbox.max.y = (subdivisionBBox.min.y + ((y + 1) / levelSizef) * bboxSize.y);

    if (level < heightChainLevelCount)
    {
        const HeightChainElement& hce = heightChain[GetChainIndex(level, x, y)];
        bbox.min.z = (subdivisionBBox.min.z + float32(hce.min) / Heightmap::MAX_VALUE * bboxSize.z);
        bbox.max.z = (subdivisionBBox.min.z + float32(hce.max) / Heightmap::MAX_VALUE * bboxSize.z);
    }
    else
    {
        //if patch size less than minimal height-map quad
        //we interpolate patch-corners inside height-map quad
        //to find exact patch bbox

        //level-difference of patch level and max level of height-map
        uint32 dLevel = level - heightChainLevelCount + 1;

        //coordinates of height-map quad origin (heightmap space)
        uint32 hx = x >> dLevel;
        uint32 hy = y >> dLevel;

        //heights of height-map quad
        float32 h00 = float32(heightmap->GetHeight(hx, hy));
        float32 h10 = float32(heightmap->GetHeightClamp(hx + 1, hy));
        float32 h01 = float32(heightmap->GetHeightClamp(hx, hy + 1));
        float32 h11 = float32(heightmap->GetHeightClamp(hx + 1, hy + 1));

        //coordinates of height-map quad origin (subdivision level space)
        uint32 ox = (x >> dLevel) << dLevel;
        uint32 oy = (y >> dLevel) << dLevel;

        //coordinates of patch corners relative to height-map quad (in range [0, 1])
        float32 dLevelScale = 1.f / GetLevelSize(dLevel);
        float32 x0 = (x - ox) * dLevelScale;
        float32 y0 = (y - oy) * dLevelScale;
        float32 x1 = x0 + dLevelScale;
        float32 y1 = y0 + dLevelScale;

        //find interpolated heights of patch corners
        float32 heights[4] = {
            InterpolateHeight(x0, y0, h00, h10, h01, h11),
            InterpolateHeight(x0, y1, h00, h10, h01, h11),
            InterpolateHeight(x1, y0, h00, h10, h01, h11),
            InterpolateHeight(x1, y1, h00, h10, h01, h11),
        };
        //find 'min' and 'max' height
        std::sort(std::begin(heights), std::end(heights));

        bbox.min.z = (subdivisionBBox.min.z + heights[0] / Heightmap::MAX_VALUE * bboxSize.z);
        bbox.max.z = (subdivisionBBox.min.z + heights[3] / Heightmap::MAX_VALUE * bboxSize.z);
    }

    bbox.min.z -= patchBBoxGapMin;
    bbox.max.z += patchBBoxGapMax;

    return bbox;
}

float32 LandscapeSubdivision::GetSubdivisionDistance(uint32 level) const
{
    float32 radiusError = metrics.normalMaxPatchRadiusError;
    float32 tanFov = tanf(metrics.normalFov * PI / 360.f);

    float32 patchSize = subdivisionBBox.GetSize().x / (1 << level);
    float32 patchRadius = patchSize * sqrtf(0.5f);

    return patchRadius / (radiusError * tanFov);
}
}
