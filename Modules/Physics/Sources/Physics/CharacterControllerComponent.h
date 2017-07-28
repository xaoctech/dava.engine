#pragma once

#include <Entity/Component.h>

#include <Reflection/Reflection.h>

namespace physx
{
class PxController;
}

namespace DAVA
{
/** Class responsible for controlling a character. Forces aren't applied to it, and it moves only when `Move` or `SimpleMove` function is called. */
class CharacterControllerComponent : public Component
{
public:
    /** Try move a character for specified `displacement`. Does not apply gravity. */
    void Move(Vector3 displacement);

    /** Try move a character for specified `displacement`. Applies gravity, any translation along up direction is ignored. */
    void SimpleMove(Vector3 displacement);

    /** Teleports a character to specified `worldPosition` */
    void Teleport(Vector3 worldPosition);

    /** Return `true` if object is touching ground, `false` otherwise */
    bool IsGrounded() const;

    /** Return character's up direction */
    Vector3 GetUpDirection() const;

    /** Set character's up direction */
    void SetUpDirection(Vector3 newUpDirection);

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
    void ScheduleUpdate();

    virtual void CopyFieldsToComponent(CharacterControllerComponent* dest);

private:
    friend class PhysicsSystem; // For accessing underlying PxController

    void SyncEntityTransform();

    physx::PxController* controller = nullptr;
    Vector3 upDirection = Vector3::UnitZ;
    bool grounded = false;

    DAVA_VIRTUAL_REFLECTION(CharacterControllerComponent, Component);
};
} // namespace DAVA