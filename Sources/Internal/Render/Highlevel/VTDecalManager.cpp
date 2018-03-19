#include "VTDecalManager.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
static const uint32 VT_EMPTY_LEAF = uint32(-1);

VTDecalManager::VTDecalManager(uint32 cellsCount_)
    : cellsCount(cellsCount_)
{
    leafIndices.resize(cellsCount * cellsCount, VT_EMPTY_LEAF);
}

VTDecalManager::VTSpaceBox VTDecalManager::GetVTSpaceBox(Vector2 min, Vector2 max)
{
    VTSpaceBox res;
    Vector2 vtMin = (min - vtSpace.min) * rcpVtCellSize;
    Vector2 vtMax = (max - vtSpace.min) * rcpVtCellSize;
    res.minx = Min(cellsCount - 1, uint32(Max(0, (int32(floor(vtMin.x))))));
    res.miny = Min(cellsCount - 1, uint32(Max(0, (int32(floor(vtMin.y))))));
    res.maxx = Min(cellsCount - 1, uint32(Max(0, (int32(floor(vtMax.x))))));
    res.maxy = Min(cellsCount - 1, uint32(Max(0, (int32(floor(vtMax.y))))));
    return res;
}
void VTDecalManager::AddDecal(DecalRenderObject* ro)
{
    ro->SetTreeNodeIndex(decals.size());
    decals.push_back(ro);
    testedDecals.Resize(decals.size());
    if (worldInitialized)
    {
        InvalidateVTPages(ro->GetWorldBoundingBox());
        const AABBox3& decalBox = ro->GetWorldBoundingBox();
        VTSpaceBox vtSpaceBox = GetVTSpaceBox(decalBox.min.xy(), decalBox.max.xy());
        for (uint32 y = vtSpaceBox.miny; y <= vtSpaceBox.maxy; ++y)
            for (uint32 x = vtSpaceBox.minx; x <= vtSpaceBox.maxx; ++x)
            {
                uint32 node = x + y * cellsCount;
                AddObjectToNode(node, ro);
            }
    }
}
void VTDecalManager::RemoveDecal(DecalRenderObject* ro)
{
    uint32 index = FindAndRemoveExchangingWithLastIndex(decals, ro);
    DVASSERT(index != static_cast<uint32>(-1));
    decals[index]->SetTreeNodeIndex(index);
    if (worldInitialized)
    {
        InvalidateVTPages(ro->GetWorldBoundingBox());
        const AABBox3& decalBox = ro->GetWorldBoundingBox();
        VTSpaceBox vtSpaceBox = GetVTSpaceBox(decalBox.min.xy(), decalBox.max.xy());
        for (uint32 y = vtSpaceBox.miny; y <= vtSpaceBox.maxy; ++y)
            for (uint32 x = vtSpaceBox.minx; x <= vtSpaceBox.maxx; ++x)
            {
                uint32 node = x + y * cellsCount;
                RemoveObjectFromNode(node, ro);
            }
    }
}

void VTDecalManager::DecalUpdated(DecalRenderObject* renderObject, const AABBox3& prevBox)
{
    const AABBox3& decalBox = renderObject->GetWorldBoundingBox();
    VTSpaceBox currVTBox = GetVTSpaceBox(decalBox.min.xy(), decalBox.max.xy());
    VTSpaceBox prevVTBox = GetVTSpaceBox(prevBox.min.xy(), prevBox.max.xy());

    //remove boxes that are in prev but not in current
    for (uint32 y = prevVTBox.miny; y <= prevVTBox.maxy; ++y)
        for (uint32 x = prevVTBox.minx; x <= prevVTBox.maxx; ++x)
        {
            if (!currVTBox.IsInside(x, y))
            {
                uint32 node = x + y * cellsCount;
                RemoveObjectFromNode(node, renderObject);
            }
        }

    //add boxes that are in current but not in prev
    for (uint32 y = currVTBox.miny; y <= currVTBox.maxy; ++y)
        for (uint32 x = currVTBox.minx; x <= currVTBox.maxx; ++x)
        {
            if (!prevVTBox.IsInside(x, y))
            {
                uint32 node = x + y * cellsCount;
                AddObjectToNode(node, renderObject);
            }
        }
}

void VTDecalManager::AddLandscape(Landscape* ro)
{
    landscapes.push_back(ro);
    if (worldInitialized)
        ReInitSpatialStructure();
}
void VTDecalManager::RemoveLandscape(Landscape* ro)
{
    FindAndRemoveExchangingWithLast(landscapes, ro);
    if (worldInitialized)
        ReInitSpatialStructure();
}

void VTDecalManager::Clip(const AABBox2& box, Vector<DecalRenderObject*>& clipResult)
{
    testedDecals.Clear();
    const float32 decalSquareThreshold = 0.1f;
    float32 thresholdBoxSize = (box.max.x - box.min.x) * (box.max.y - box.min.y) * decalSquareThreshold;
    VTSpaceBox vtSpaceBox = GetVTSpaceBox(box.min, box.max);
    for (uint32 y = vtSpaceBox.miny; y <= vtSpaceBox.maxy; ++y)
        for (uint32 x = vtSpaceBox.minx; x <= vtSpaceBox.maxx; ++x)
        {
            uint32 node = x + y * cellsCount;
            if (leafIndices[node] != VT_EMPTY_LEAF)
            {
                for (DecalRenderObject* decal : leafs[leafIndices[node]])
                {
                    uint32 decalIndex = decal->GetTreeNodeIndex();
                    if (testedDecals.At(decalIndex))
                        continue;
                    testedDecals.Set(decalIndex, true);
                    const AABBox3& decalBox = decal->GetWorldBoundingBox();
                    float32 decalSize = (decalBox.max.x - decalBox.min.x) * (decalBox.max.y - decalBox.min.y);
                    if (decalSize < thresholdBoxSize) //skip
                        continue;
                    if (box.IsIntersectsWithBox(AABBox2(decalBox.min.xy(), decalBox.max.xy())))
                    {
                        if (decal->GetSplineData() == nullptr)
                            clipResult.push_back(decal);
                        else
                        {
                            //do fine grain clipping
                            for (size_t i = 0, sz = decal->GetSplineData()->segmentData.size(); i < sz; ++i)
                            {
                                //GFX_COMPLETE - fill data for partial draw?
                                if (box.IsIntersectsWithBox(decal->GetSplineData()->segmentData[i].worldBbox))
                                {
                                    clipResult.push_back(decal);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
}

void VTDecalManager::Initialize()
{
    worldInitialized = true;
    ReInitSpatialStructure();
}

void VTDecalManager::ReInitSpatialStructure()
{
    //clear existing spatial data
    for (uint32 i = 0, sz = static_cast<uint32>(leafIndices.size()); i < sz; ++i)
    {
        if (leafIndices[i] != VT_EMPTY_LEAF)
        {
            leafs[leafIndices[i]].clear();
            ReleaseLeaf(leafIndices[i]);
        }
    }
    //create vtSpace for all landscapes
    if (landscapes.empty())
    {
        vtSpace = AABBox2(Vector2(-1.0f, -1.0f), Vector2(1.0f, 1.0f));
    }
    else
    {
        vtSpace.Empty();
        for (Landscape* landscape : landscapes)
        {
            const AABBox3& landscapeBox = landscape->GetWorldBoundingBox();
            vtSpace.AddPoint(landscapeBox.min.xy());
            vtSpace.AddPoint(landscapeBox.max.xy());
        }
    }
    //corner case with some empty landscapes
    if ((vtSpace.max.x - vtSpace.min.x) < 0.1)
        vtSpace.max.x += 0.1f;
    if ((vtSpace.max.y - vtSpace.min.y) < 0.1)
        vtSpace.max.y += 0.1f;

    rcpVtCellSize = Vector2(cellsCount / (vtSpace.max.x - vtSpace.min.x), cellsCount / (vtSpace.max.x - vtSpace.min.x));
    //put existing decals int corresponding lists
    for (DecalRenderObject* ro : decals)
    {
        const AABBox3& decalBox = ro->GetWorldBoundingBox();
        VTSpaceBox vtSpaceBox = GetVTSpaceBox(decalBox.min.xy(), decalBox.max.xy());
        for (uint32 y = vtSpaceBox.miny; y <= vtSpaceBox.maxy; ++y)
            for (uint32 x = vtSpaceBox.minx; x <= vtSpaceBox.maxx; ++x)
            {
                uint32 node = x + y * cellsCount;
                AddObjectToNode(node, ro);
            }
    }
}

uint32 VTDecalManager::AllocLeaf()
{
    if (freeLeafs.size())
    {
        uint32 leaf = freeLeafs.back();
        freeLeafs.pop_back();
        return leaf;
    }

    leafs.resize(leafs.size() + 1);
    return static_cast<uint32>(leafs.size() - 1);
}
void VTDecalManager::ReleaseLeaf(uint32 leaf)
{
    DVASSERT(leafs[leaf].empty());
    freeLeafs.push_back(leaf);
}

void VTDecalManager::AddObjectToNode(uint32 node, DecalRenderObject* ro)
{
    if (leafIndices[node] == VT_EMPTY_LEAF)
        leafIndices[node] = AllocLeaf();
    leafs[leafIndices[node]].push_back(ro);
}
void VTDecalManager::RemoveObjectFromNode(uint32 node, DecalRenderObject* ro)
{
    DVASSERT(leafIndices[node] != VT_EMPTY_LEAF);
    DVASSERT(leafIndices[node] < leafs.size());
    bool decalFound = FindAndRemoveExchangingWithLast(leafs[leafIndices[node]], ro);
    DVASSERT(decalFound);
    if (leafs[leafIndices[node]].empty())
    {
        ReleaseLeaf(leafIndices[node]);
        leafIndices[node] = VT_EMPTY_LEAF;
    }
}

void VTDecalManager::PrepareForShutdown()
{
}

void VTDecalManager::InvalidateVTPages(const AABBox3& worldBox)
{
    for (Landscape* landscape : landscapes)
    {
        const AABBox3& landscapeBox = landscape->GetWorldBoundingBox();
        if (landscapeBox.IntersectsWithBox(worldBox))
        {
            Vector2 rcpSize = Vector2(1.0f / (landscapeBox.max.x - landscapeBox.min.x), 1.0f / (landscapeBox.max.x - landscapeBox.min.x));
            Vector2 base = (worldBox.min.xy() - landscapeBox.min.xy()) * rcpSize;
            Vector2 size = worldBox.GetSize().xy() * rcpSize;
            landscape->InvalidatePages(Rect(base, size));
        }
    }
}

void VTDecalManager::DebugDraw(RenderHelper* renderHelper)
{
    Color colorObject(0.2f, 0.2f, 1.0f, 1.0f);
    Color colorNode(0.2f, 1.0f, 0.2f, 1.0f);
    Color colorNodeEmpty(0.1f, 0.4f, 0.1f, 1.0f);
    float32 boxH = 1.0;

    Vector2 vtCellSize = 1.0f / rcpVtCellSize;
    for (uint32 y = 0; y < cellsCount; y++)
        for (uint32 x = 0; x < cellsCount; x++)
        {
            uint32 node = x + y * cellsCount;
            Vector2 boxLeft = Vector2(float32(x), float32(y)) * vtCellSize + vtSpace.min;
            Vector2 boxCenter = boxLeft + 0.5f * vtCellSize;
            Vector2 boxRight = boxLeft + vtCellSize;
            Vector3 worldPosCenter = Vector3(boxCenter.x, boxCenter.y, 0.0f);
            for (Landscape* landscape : landscapes)
            {
                if (landscape->PlacePoint(worldPosCenter, worldPosCenter))
                    break;
            }
            AABBox3 worldBox = AABBox3(Vector3(boxLeft.x, boxLeft.y, worldPosCenter.z - boxH), Vector3(boxRight.x, boxRight.y, worldPosCenter.z + boxH));
            renderHelper->DrawAABox(worldBox, leafIndices[node] == VT_EMPTY_LEAF ? colorNodeEmpty : colorNode, RenderHelper::DRAW_WIRE_DEPTH);
        }
    for (DecalRenderObject* decal : decals)
    {
        renderHelper->DrawAABox(decal->GetWorldBoundingBox(), colorObject, RenderHelper::DRAW_WIRE_DEPTH);
    }
}
}