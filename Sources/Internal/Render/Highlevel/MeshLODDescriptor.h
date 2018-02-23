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
        .Field("material", &MeshBatchDescriptor::materialPath)
        .Field("switchIndex", &MeshBatchDescriptor::switchIndex)
        .Field("geometryIndex", &MeshBatchDescriptor::geometryIndex)
        .End();
    }
};

struct MeshLODDescriptor : public ReflectionBase
{
    FilePath geometryPath;
    int32 lodIndex = -1;
    Vector<MeshBatchDescriptor> batchDescriptors;

    Asset<Geometry> geometryAsset;

    bool operator==(const MeshLODDescriptor& other) const
    {
        return geometryPath == other.geometryPath &&
        lodIndex == other.lodIndex &&
        batchDescriptors == other.batchDescriptors;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MeshLODDescriptor, ReflectionBase)
    {
        ReflectionRegistrator<MeshLODDescriptor>::Begin()
        .Field("geometryPath", &MeshLODDescriptor::geometryPath)
        .Field("lodIndex", &MeshLODDescriptor::lodIndex)
        .Field("batchDescriptors", &MeshLODDescriptor::batchDescriptors)
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
