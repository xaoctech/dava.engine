#pragma once

#include <Base/FastName.h>
#include <Entity/Component.h>
#include <Scene3D/Entity.h>

#include <Reflection/Reflection.h>

#include <physx/PxFiltering.h>

namespace physx
{
class PxShape;
}

namespace DAVA
{
class CollisionShapeComponent : public Component
{
public:
    enum JointSyncDirection : uint32
    {
        FromPhysics,
        ToPhysics
    };

public:
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    physx::PxShape* GetPxShape() const;

    const FastName& GetName() const;
    void SetName(const FastName& name);

    const Vector3& GetLocalPosition() const;
    void SetLocalPosition(const Vector3& value);

    const Quaternion& GetLocalOrientation() const;
    void SetLocalOrientation(const Quaternion& value);

    bool GetOverrideMass() const;
    void SetOverrideMass(bool override);

    float32 GetMass() const;
    void SetMass(float32 mass);

    void SetTypeMask(uint32 typeMask);
    uint32 GetTypeMask() const;

    void SetTypeMaskToCollideWith(uint32 typeMaskToCollideWith);
    uint32 GetTypeMaskToCollideWith() const;

    const FastName& GetMaterialName() const;
    void SetMaterialName(const FastName& materialName);

    void SetTriggerMode(bool isEnabled);
    bool GetTriggerMode() const;

    const FastName& GetJointName() const;
    void SetJointName(const FastName& value);

    JointSyncDirection GetJointSyncDirection() const;
    void SetJointSyncDirection(JointSyncDirection value);

    Vector3 GetJointOffset() const;
    void SetJointOffset(const Vector3& value);

    static CollisionShapeComponent* GetComponent(const physx::PxShape* shape);
    static bool IsCCDEnabled(const physx::PxFilterData& filterData);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    virtual void CheckShapeType() const = 0;
#endif

    void CopyFieldsIntoClone(CollisionShapeComponent* component) const;
    void ScheduleUpdate();
    void UpdateFilterData();
    virtual void UpdateLocalProperties();

    Vector3 localPosition = Vector3::Zero;
    Quaternion localOrientation = Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
    Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);

    virtual void ReleasePxShape();

private:
    FastName name = FastName("");
    bool overrideMass = false;
    float32 mass = 1.0f;
    uint32 typeMask = 0;
    uint32 typeMaskToCollideWith = 0;
    FastName materialName;
    bool triggerMode = false;
    FastName jointName;
    JointSyncDirection jointSyncDirection;
    Vector3 jointOffset;

    physx::PxShape* shape = nullptr;

    friend class PhysicsSystem;
    void SetPxShape(physx::PxShape* shape);
    static void SetCCDEnabled(physx::PxShape* shape, bool isCCDActive);

    DAVA_VIRTUAL_REFLECTION(CollisionShapeComponent, Component);
};
} // namespace DAVA
