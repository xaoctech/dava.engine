#pragma once

#include <Entity/Component.h>

#include <Reflection/Reflection.h>

namespace physx
{
class PxController;
class PxActor;
}

namespace DAVA
{
/**
    Class responsible for controlling a character.
    This component should only be attached to root entities (i.e. entities on a highest level of hierarchy).
*/
class CharacterControllerComponent : public Component
{
public:
    /** Enum describing different movement scehemes. */
    enum MovementMode
    {
        /** Gravity applies to a character, any displacement along up axis is ignored. */
        Walking,

        /** Gravity does not affect a character, free movement in any direction. */
        Flying
    };

    /** Set character's contact offset. */
    void SetContactOffset(float32 value);

    /** Get character's contact offset. */
    float32 GetContactOffset() const;

    /** Set character's scale coefficient. */
    void SetScaleCoeff(float32 value);

    /** Get character's scale coefficient. */
    float32 GetScaleCoeff() const;

    /** Set character's movement mode. */
    void SetMovementMode(MovementMode newMode);

    /** Get character's movement mode. */
    MovementMode GetMovementMode() const;

    /** Get character's offset for the next update. **/
    const Vector3& GetOffset() const;

    /** Set character's offset for the next update. **/
    void SetOffset(const Vector3& value);

    /** Get character's velocity for the next update. **/
    const Vector3& GetVelocity() const;

    /** Set character's velocity for the next update. **/
    void SetVelocity(const Vector3& value);

    /** Teleport a character to specified `worldPosition`. */
    void Teleport(const Vector3& worldPosition);

    /** Return `true` if object is touching the ground, `false` otherwise. */
    bool IsGrounded() const;

    void SetTypeMask(uint32 value);
    uint32 GetTypeMask() const;

    void SetTypeMaskToCollideWith(uint32 value);
    uint32 GetTypeMaskToCollideWith() const;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    physx::PxActor* GetPxActor() const;
    physx::PxController* GetPxController() const;

protected:
    void ScheduleUpdate();

    virtual void CopyFieldsToComponent(CharacterControllerComponent* dest);

protected:
    bool geometryChanged = false;

private:
    friend class PhysicsSystem;

    physx::PxController* controller = nullptr;

    float32 contactOffset = 0.1f;
    float32 scaleCoeff = 0.8f;

    MovementMode mode = MovementMode::Walking;

    bool grounded = false;

    Vector3 totalDisplacement = Vector3::Zero;
    Vector3 totalVelocity = Vector3::Zero;

    bool teleported = false;
    Vector3 teleportDestination = Vector3::Zero;

    uint32 typeMask = 0;
    uint32 typeMaskToCollideWith = 0;

    DAVA_VIRTUAL_REFLECTION(CharacterControllerComponent, Component);
};
} // namespace DAVA
