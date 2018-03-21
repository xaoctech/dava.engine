#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Render/Highlevel/MeshLODDescriptor.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Asset/AssetListener.h"

namespace DAVA
{
class SpeedTreeObject;
class SpeedTreeUpdateSystem;
class SpeedTreeComponent : public Component, public AssetListener
{
public:
    enum Element : uint32
    {
        Trunk,
        Branch,
        Leaves,
        FacingLeaves,

        Count,
    };

    struct Bone
    {
        int32 id = -1;
        int32 parentId = -1;
        Element element = Element::Trunk;
        Vector3 start;
        Vector3 end;
        float32 mass = 1.0f;
        float32 massWithChildren = 1.0f;
        float32 radius = 0.0f;

        void Serialize(KeyedArchive*);
        void Deserialize(KeyedArchive*);
    };

    struct RuntimeBone
    {
        Vector3 angles = Vector3(0.0f, 0.0f, 0.0f);
        Vector3 velocities = Vector3(0.0f, 0.0f, 0.0f);
    };

public:
    SpeedTreeComponent();
    ~SpeedTreeComponent();

    void SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc);
    const Vector<MeshLODDescriptor>& GetMeshDescriptor() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    SpeedTreeObject* GetSpeedTreeObject() const;

    const Vector<Bone>& GetBones() const;
    void SetBones(const Vector<Bone>& bones);

    float GetFlexibility(Element) const;
    void SetFlexibility(Element el, float f);

    float GetTrunkFlexibility() const;
    void SetTrunkFlexibility(const float&);

    float GetBranchesFlexibility() const;
    void SetBranchesFlexibility(const float&);

    float GetLeavesFlexibility() const;
    void SetLeavesFlexibility(const float&);

    float GetTrunkDamping() const;

    Vector<RuntimeBone>& GetRuntimeBones();

    void OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloaded) override;

protected:
    void RebuildObject();

    Vector<MeshLODDescriptor> meshLODDescriptors;
    Vector<Bone> bones;

    //runtime
    Vector<RuntimeBone> runtime;
    Matrix4 worldTransfromInv;
    float32 flexibility[Element::Count];
    float32 trunkDamping = 1.0f;

    Vector4 wind;
    Vector4 leafsFlexibility;

    SpeedTreeObject* treeObject = nullptr;

    DAVA_VIRTUAL_REFLECTION(SpeedTreeComponent, Component);

    friend class SpeedTreeUpdateSystem;
};

inline const Vector<SpeedTreeComponent::Bone>& SpeedTreeComponent::GetBones() const
{
    return bones;
}

inline float SpeedTreeComponent::GetFlexibility(Element el) const
{
    return flexibility[el];
}
inline void SpeedTreeComponent::SetFlexibility(Element el, float f)
{
    flexibility[el] = f;
    for (RuntimeBone& bone : runtime)
    {
        bone.angles = Vector3(0.0f, 0.0f, 0.0f);
        bone.velocities = Vector3(0.0f, 0.0f, 0.0f);
    }
}
inline float SpeedTreeComponent::GetTrunkFlexibility() const
{
    return GetFlexibility(Element::Trunk);
}
inline void SpeedTreeComponent::SetTrunkFlexibility(const float& f)
{
    SetFlexibility(Element::Trunk, f);
}
inline float SpeedTreeComponent::GetBranchesFlexibility() const
{
    return GetFlexibility(Element::Branch);
}
inline void SpeedTreeComponent::SetBranchesFlexibility(const float& f)
{
    SetFlexibility(Element::Branch, f);
}
inline float SpeedTreeComponent::GetLeavesFlexibility() const
{
    return GetFlexibility(Element::Leaves);
}
inline void SpeedTreeComponent::SetLeavesFlexibility(const float& f)
{
    SetFlexibility(Element::Leaves, f);
}
inline Vector<SpeedTreeComponent::RuntimeBone>& SpeedTreeComponent::GetRuntimeBones()
{
    return runtime;
}
inline float SpeedTreeComponent::GetTrunkDamping() const
{
    return trunkDamping;
}
}
