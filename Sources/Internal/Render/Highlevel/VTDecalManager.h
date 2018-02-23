#pragma once

#include <Math/AABBox3.h>

namespace DAVA
{
class DecalRenderObject;
class Landscape;
class RenderHelper;
class VTDecalManager
{
public:
    //cereate 2d cellsCount x cellsCount grid with size
    VTDecalManager(uint32 cellsCount);
    void AddDecal(DecalRenderObject* ro);
    void RemoveDecal(DecalRenderObject* ro);
    void DecalUpdated(DecalRenderObject* renderObject, const AABBox3& prevBox);
    void Clip(const AABBox2& box, Vector<DecalRenderObject*>& clipResult);

    void AddLandscape(Landscape* ro);
    void RemoveLandscape(Landscape* ro);

    void Initialize();
    void PrepareForShutdown();
    void InvalidateVTPages(const AABBox3& worldBox);

    void DebugDraw(RenderHelper* renderHelper);

private:
    struct VTSpaceBox
    {
        uint32 minx, miny, maxx, maxy;
        bool IsInside(uint32 x, uint32 y); //for intersection checking
    };
    void ReInitSpatialStructure();
    uint32 AllocLeaf();
    void ReleaseLeaf(uint32 leaf);
    void AddObjectToNode(uint32 node, DecalRenderObject* ro);
    void RemoveObjectFromNode(uint32 node, DecalRenderObject* ro);
    VTSpaceBox GetVTSpaceBox(Vector2 min, Vector2 max);

private:
    Vector<DecalRenderObject*> decals;
    Vector<Landscape*> landscapes;
    bool worldInitialized = false;

    uint32 cellsCount;
    AABBox2 vtSpace;
    Vector2 rcpVtCellSize; //small optimization

    Vector<uint32> leafIndices;
    Vector<Vector<DecalRenderObject*>> leafs;
    Vector<uint32> freeLeafs;
};

inline bool VTDecalManager::VTSpaceBox::IsInside(uint32 x, uint32 y)
{
    return (x >= minx) && (x <= maxx) && (y >= miny) && (y <= maxy);
}
}
