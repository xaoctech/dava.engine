#include "VTDecalManager.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Render/Highlevel/Landscape.h"

namespace DAVA
{
VTDecalManager::VTDecalManager(int32 cellsCount)
{
}
void VTDecalManager::AddDecal(DecalRenderObject* ro)
{
    decals.push_back(ro);
    InvalidateVTPages(ro->GetWorldBoundingBox());
}
void VTDecalManager::RemoveDecal(DecalRenderObject* ro)
{
    InvalidateVTPages(ro->GetWorldBoundingBox());
    FindAndRemoveExchangingWithLast(decals, ro);
}

void VTDecalManager::AddLandscape(Landscape* ro)
{
    landscapes.push_back(ro);
}
void VTDecalManager::RemoveLandscape(Landscape* ro)
{
    FindAndRemoveExchangingWithLast(landscapes, ro);
}

void VTDecalManager::DecalUpdated(DecalRenderObject* renderObject)
{
}

void VTDecalManager::Clip(const AABBox2& box, Vector<DecalRenderObject*>& clipResult)
{
    for (DecalRenderObject* decal : decals)
    {
        const AABBox3& decalBox = decal->GetWorldBoundingBox();
        if (box.IsIntersectsWithBox(AABBox2(decalBox.min.xy(), decalBox.max.xy())))
        {
            if (decal->GetSplineData() == nullptr)
                clipResult.push_back(decal);
            else
            {
                //GFX_COMPLETE
                //clipResult.push_back(decal);
                //do fine grain clipping
                for (size_t i = 0, sz = decal->GetSplineData()->segmentData.size(); i < sz; ++i)
                {
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

void VTDecalManager::Initialize()
{
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
}