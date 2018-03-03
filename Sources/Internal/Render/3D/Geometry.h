#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/3D/PolygonGroup.h"
#include "Asset/Asset.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class Geometry : public AssetBase
{
public:
    Geometry(const Any& assetKey);
    ~Geometry();

    void AddPolygonGroup(PolygonGroup* pgroup);
    uint32 GetPolygonGroupCount() const;
    PolygonGroup* GetPolygonGroup(uint32 index) const;

protected:
    friend class GeometryAssetLoader;
    Vector<PolygonGroup*> geometries;

    DAVA_VIRTUAL_REFLECTION(Geometry, AssetBase);
};
};
