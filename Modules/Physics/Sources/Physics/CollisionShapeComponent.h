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

protected:
#if defined(__DAVAENGINE_DEBUG__)
    virtual void CheckShapeType() const = 0;
#endif

    void CopyFields(CollisionShapeComponent* component);

private:
    FastName name = FastName("");
    Matrix4 localPose;

    physx::PxShape* shape = nullptr;

    friend class PhysicsSystem;
    void SetPxShape(physx::PxShape* shape);
    void ReleasePxShape();

    DAVA_VIRTUAL_REFLECTION(CollisionShapeComponent, Component);
};
} // namespace DAVA