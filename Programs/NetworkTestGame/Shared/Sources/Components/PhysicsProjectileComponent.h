#pragma once

#include <Base/BaseTypes.h>
#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
}

class PhysicsProjectileComponent final : public DAVA::Component
{
public:
    enum eProjectileTypes
    {
        MISSILE,
        GRENADE
    };

    enum eProjectileStates
    {
        FLYING,
        DETONATION_TIMING,
        DESTROYED,
    };

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
    void Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext) override;
    void Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext) override;

    eProjectileTypes GetProjectileType() const;
    void SetProjectileType(eProjectileTypes value);

    eProjectileStates GetProjectileState() const;
    void SetProjectileState(eProjectileStates value);

    void SetInitialPosition(const DAVA::Vector3& value);
    DAVA::Vector3 GetInitialPosition() const;

    DAVA_VIRTUAL_REFLECTION(PhysicsProjectileComponent, Component);

private:
    eProjectileTypes type = eProjectileTypes::MISSILE;
    eProjectileStates state = eProjectileStates::FLYING;
    DAVA::Vector3 initialPos = DAVA::Vector3::Zero;
};
