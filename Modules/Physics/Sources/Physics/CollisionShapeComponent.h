#pragma once

#include <Base/FastName.h>
#include <Entity/Component.h>
#include <Scene3D/Entity.h>

#include <Reflection/Reflection.h>

namespace physx
{
class PxShape;
} // namespace physx

namespace DAVA
{
class CollisionShapeComponent : public Component
{
public:
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    physx::PxShape* GetPxShape() const;

    const FastName& GetName() const;
    void SetName(const FastName& name);

    const Matrix4& GetLocalPose() const;
    void SetLocalPose(const Matrix4& localPose);

    bool GetOverrideMass() const;
    void SetOverrideMass(bool override);

    float32 GetMass() const;
    void SetMass(float32 mass);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    virtual void CheckShapeType() const = 0;
#endif

    void CopyFields(CollisionShapeComponent* component);
    void SheduleUpdate();
    virtual void UpdateLocalProperties();

    Matrix4 localPose;
    Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);

private:
    FastName name = FastName("");
    bool overrideMass = false;
    float32 mass = 1.0f;

    physx::PxShape* shape = nullptr;

    friend class PhysicsSystem;
    void SetPxShape(physx::PxShape* shape);
    void ReleasePxShape();

    DAVA_VIRTUAL_REFLECTION(CollisionShapeComponent, Component);
};
} // namespace DAVA