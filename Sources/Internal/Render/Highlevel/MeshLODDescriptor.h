#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "FileSystem/FilePath.h"
#include "Render/3D/Geometry.h"
#include "Render/Material/Material.h"

namespace DAVA
{
class KeyedArchive;
class SerializationContext;
struct MeshBatchDescriptor : public ReflectionBase
{
    FilePath materialPath;
    Vector<int32> jointTargets;
    uint32 geometryIndex = 0;
    int32 switchIndex = -1;

    Asset<Material> materialAsset;

    const FilePath& GetMaterialPath() const;
    void SetMaterialPath(const FilePath& path);

    bool operator==(const MeshBatchDescriptor& other) const
    {
        return materialPath == other.materialPath &&
        jointTargets == other.jointTargets &&
        geometryIndex == other.geometryIndex &&
        switchIndex == other.switchIndex;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MeshBatchDescriptor, ReflectionBase)
    {
        ReflectionRegistrator<MeshBatchDescriptor>::Begin()
        .Field("material", &MeshBatchDescriptor::GetMaterialPath, &MeshBatchDescriptor::SetMaterialPath)[M::DisplayName("Material")]
        .Field("switchIndex", &MeshBatchDescriptor::switchIndex)[M::DisplayName("Switch index"), M::Range(-1, 1, 1)]
        .Field("geometryIndex", &MeshBatchDescriptor::geometryIndex)[M::DisplayName("Geometry index"), M::ReadOnly(), M::DeveloperModeOnly()]
        .End();
    }
};

struct MeshLODDescriptor : public ReflectionBase
{
    FilePath geometryPath;
    int32 lodIndex = -1;
    Vector<MeshBatchDescriptor> batchDescriptors;

    Asset<Geometry> geometryAsset;

    const FilePath& GetGeometryPath() const;
    void SetGeometryPath(const FilePath& path);
    void SetGeometry(const Asset<Geometry>& geometry);

    bool operator==(const MeshLODDescriptor& other) const
    {
        return geometryPath == other.geometryPath &&
        lodIndex == other.lodIndex &&
        batchDescriptors == other.batchDescriptors;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MeshLODDescriptor, ReflectionBase)
    {
        ReflectionRegistrator<MeshLODDescriptor>::Begin()
        .Field("geometry", &MeshLODDescriptor::GetGeometryPath, &MeshLODDescriptor::SetGeometryPath)[M::DisplayName("Geometry")]
        .Field("lodIndex", &MeshLODDescriptor::lodIndex)[M::DisplayName("Lod index"), M::Range(-1, 3, 1)]
        .Field("batchDescriptors", &MeshLODDescriptor::batchDescriptors)[M::DisplayName("Batches"), M::ReadOnly()]
        .End();
    }

    static void Serialize(const Vector<MeshLODDescriptor>& meshLODDescriptors, KeyedArchive* archive, SerializationContext* serializationContext);
    static void Deserialize(Vector<MeshLODDescriptor>* meshLODDescriptors, KeyedArchive* archive, SerializationContext* serializationContext);
};

template <>
struct AnyCompare<MeshBatchDescriptor>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<MeshBatchDescriptor>() == v2.Get<MeshBatchDescriptor>();
    }
};

template <>
struct AnyCompare<MeshLODDescriptor>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<MeshLODDescriptor>() == v2.Get<MeshLODDescriptor>();
    }
};

} // ns
