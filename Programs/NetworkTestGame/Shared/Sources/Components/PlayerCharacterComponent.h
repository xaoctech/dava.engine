#pragma once

#include "Entity/Component.h"
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>

#include "NetworkCore/NetworkTypes.h"

class PlayerCharacterComponent : public DAVA::Component
{
    DAVA_VIRTUAL_REFLECTION(PlayerTankComponent, DAVA::Component);

public:
    PlayerCharacterComponent() = default;
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::NetworkPlayerID playerId;
};
