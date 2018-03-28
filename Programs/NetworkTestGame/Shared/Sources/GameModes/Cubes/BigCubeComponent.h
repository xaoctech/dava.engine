#pragma once

#include <NetworkCore/NetworkTypes.h>

#include <Entity/Component.h>

namespace DAVA
{
class Entity;
} // namespace DAVA

class BigCubeComponent final : public DAVA::Component
{
    DAVA_VIRTUAL_REFLECTION(BigCubeComponent, DAVA::Component);

public:
    DAVA::Component* Clone(DAVA::Entity* toEntity) override;

    DAVA::NetworkPlayerID playerId;
};