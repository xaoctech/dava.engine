#pragma once

#include "Base/Any.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/3D/PolygonGroup.h"
#include "Asset/Asset.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class Geometry : public AssetBase
{
public:
    struct PathKey
    {
        PathKey() = default;
        PathKey(const FilePath& filepath)
            : path(filepath)
        {
        }
        FilePath path;
    };

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

template <>
bool AnyCompare<Geometry::PathKey>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<Geometry::PathKey>;
};
