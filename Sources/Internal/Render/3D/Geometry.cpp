#include "Render/3D/Geometry.h"
#include "FileSystem/File.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
Geometry::Geometry(const Any& assetKey)
    : AssetBase(assetKey)
{
}

Geometry::~Geometry()
{
    for (PolygonGroup* pg : geometries)
    {
        SafeRelease(pg);
    }
    geometries.clear();
}

void Geometry::AddPolygonGroup(PolygonGroup* pgroup)
{
    geometries.emplace_back(SafeRetain(pgroup));
}

uint32 Geometry::GetPolygonGroupCount() const
{
    return uint32(geometries.size());
}

PolygonGroup* Geometry::GetPolygonGroup(uint32 index) const
{
    DVASSERT(index < uint32(geometries.size()));
    return geometries[index];
}

DAVA_VIRTUAL_REFLECTION_IMPL(Geometry)
{
    ReflectionRegistrator<Geometry>::Begin()
    .End();
}

template <>
bool AnyCompare<Geometry::PathKey>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<Geometry::PathKey>().path == v2.Get<Geometry::PathKey>().path;
}
};
