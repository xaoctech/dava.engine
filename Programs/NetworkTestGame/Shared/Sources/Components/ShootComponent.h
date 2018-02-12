#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Entity/SceneSystem.h"

enum class ShootPhase : DAVA::uint8
{
    BURN,
    FLY,
    DESTROY,
};

class ShootComponent : public DAVA::Component
{
protected:
    virtual ~ShootComponent();

public:
    static const DAVA::uint32 MAX_DISTANCE;
    static const DAVA::float32 MOVE_SPEED;

    enum ShootType
    {
        MAIN = 1 << 0,
        STUN = 1 << 1,
    };

    ShootComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
    void Serialize(DAVA::KeyedArchive* archive,
                   DAVA::SerializationContext* serializationContext) override;
    void Deserialize(DAVA::KeyedArchive* archive,
                     DAVA::SerializationContext* serializationContext) override;

    ShootPhase GetPhase() const;
    void SetPhase(ShootPhase phase);

    DAVA::uint32 GetDistance() const;
    void SetDistance(DAVA::uint32 distance_);

    DAVA::uint32 GetShootType() const;
    void SetShootType(DAVA::uint32 shootTypeMask);

    DAVA::Entity* GetShooter() const;
    void SetShooter(DAVA::Entity* shooter);

    DAVA_VIRTUAL_REFLECTION(ShootComponent, Component);

protected:
    ShootPhase phase = ShootPhase::BURN;
    DAVA::uint32 distance = 0;
    DAVA::uint32 shootTypeMask = 0;
    DAVA::Entity* shooter = nullptr;
};
