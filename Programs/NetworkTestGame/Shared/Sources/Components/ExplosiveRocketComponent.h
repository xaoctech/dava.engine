#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Entity/SceneSystem.h"
#include <NetworkCore/NetworkTypes.h>

class ExplosiveRocketComponent : public DAVA::Component
{
protected:
    virtual ~ExplosiveRocketComponent();

public:
    static const DAVA::uint32 SPLIT_DISTANCE;
    static const DAVA::uint32 MAX_DISTANCE;
    static const DAVA::float32 MOVE_SPEED;
    static const DAVA::float32 ROT_SPEED;

    enum Stage : DAVA::uint8
    {
        BOOSTER,
        DESTROYER,
    };

    ExplosiveRocketComponent();

    DAVA::Component* Clone(DAVA::Entity* toEntity) override;
    void Serialize(DAVA::KeyedArchive* archive,
                   DAVA::SerializationContext* serializationContext) override;
    void Deserialize(DAVA::KeyedArchive* archive,
                     DAVA::SerializationContext* serializationContext) override;

    DAVA::uint8 GetStage() const;
    void SetStage(DAVA::uint8 stage);

    DAVA::uint32 GetDistance() const;
    void SetDistance(DAVA::uint32 distance_);

    DAVA_VIRTUAL_REFLECTION(ExplosiveRocketComponent, Component);

    DAVA::NetworkID shooterId;
    DAVA::NetworkID targetId;

protected:
    Stage stage = Stage::BOOSTER;
    DAVA::uint32 distance = 0;
};
