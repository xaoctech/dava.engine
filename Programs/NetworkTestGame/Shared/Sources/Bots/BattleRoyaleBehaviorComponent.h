#pragma once

#include "BotTaskComponent.h"
#include "BehaviorComponent.h"

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Entity.h>
#include <Utils/Random.h>

namespace DAVA
{
class TankBattleRoyaleBehaviorSystem;
class ShooterBattleRoyaleBehaviorSystem;

class BattleRoyaleBehaviorComponent final : public BehaviorComponent
{
public:
    BattleRoyaleBehaviorComponent() = default;
    Component* Clone(Entity* toEntity) override;

private:
    friend class TankBattleRoyaleBehaviorSystem;
    friend class ShooterBattleRoyaleBehaviorSystem;

    Random localRandom;

    DAVA_VIRTUAL_REFLECTION(BattleRoyaleBehaviorComponent, BehaviorComponent);
};
}
