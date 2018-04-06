#pragma once

#include "Base/BaseTypes.h"
#include "Animation/AnimatedObject.h"
#include "Base/BaseMath.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class PolygonGroup;
class RenderBatch;
class NMaterial;
class JointTransform;
struct MeshLODDescriptor;
class Mesh : public RenderObject
{
public:
    const static uint32 MAX_TARGET_JOINTS = 64; //same as in shader

    using JointTargets = Vector<int32>; // vector's index is joint target, value - skeleton joint index.

    struct JointTargetsData
    {
        JointTargetsData() = default;

        Vector<Vector4> positions;
        Vector<Vector4> quaternions;
        uint32 jointsDataCount = 0;
    };

    Mesh();
    virtual ~Mesh() = default;

    RenderObject* Clone(RenderObject* newObject) override;

    void SetBoundingBox(const AABBox3& box);

    void AddMeshBatches(const Vector<MeshLODDescriptor>& desc);
    void BakeGeometry(const Matrix4& transform) override;

    //Skinning
    void SetJointTargets(RenderBatch* batch, const JointTargets& jointTargets); //STREAMING_COMPLETE should be protected, after models import refactoring
    void UpdateJointTransforms(const Vector<JointTransform>& finalTransforms);

    bool HasSkinnedBatches() const;
    const JointTargets& GetJointTargets(RenderBatch* batch);
    const JointTargetsData& GetJointTargetsData(RenderBatch* batch);

    void UpdatePreviousState() override;

    //DEPRECATED
    DAVA_DEPRECATED(void AddPolygonGroup(PolygonGroup* polygonGroup, NMaterial* material));
    DAVA_DEPRECATED(void Load(KeyedArchive* archive, SerializationContext* serializationContext) override);

protected:
    //Skinning
    void UpdateJointsTransformsProperties();

    UnorderedMap<RenderBatch*, uint32> jointTargetsDataMap; //RenderBatch -> targets-data index
    Vector<std::pair<JointTargets, JointTargetsData>> jointTargetsData;
    Vector<std::pair<JointTargets, JointTargetsData>> prevJointTargetsData;

    DAVA_VIRTUAL_REFLECTION(Mesh, RenderObject);
};

} // ns
