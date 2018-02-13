#pragma once

#include <Math/AABBox3.h>

namespace DAVA
{
class DecalRenderObject;
class Landscape;
class VTDecalManager
{
public:
    //cereate 2d cellsCount x cellsCount grid with size
    VTDecalManager(int32 cellsCount);
    void AddDecal(DecalRenderObject* ro);
    void RemoveDecal(DecalRenderObject* ro);
    void DecalUpdated(DecalRenderObject* renderObject);
    void Clip(const AABBox2& box, Vector<DecalRenderObject*>& clipResult);

    void AddLandscape(Landscape* ro);
    void RemoveLandscape(Landscape* ro);

    void Initialize();
    void PrepareForShutdown();
    void InvalidateVTPages(const AABBox3& worldBox);

private:
    Vector<DecalRenderObject*> decals;
    Vector<Landscape*> landscapes;
};
}
