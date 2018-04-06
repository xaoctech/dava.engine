#include "Game.h"

#include <Reflection/ReflectionRegistrator.h>

#include "Components/ExplosionEffectComponent.h"
#include "Components/ShootCooldownComponent.h"
#include "Components/ShootComponent.h"
#include "Components/GameStunningComponent.h"
#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/DamageComponent.h"
#include "Components/PhysicsProjectileComponent.h"
#include "Components/PlayerTankComponent.h"
#include "Components/PlayerCarComponent.h"
#include "Components/PlayerCharacterComponent.h"
#include "Components/PlayerInvaderComponent.h"
#include "Components/ShooterRoleComponent.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterProjectileComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "Components/PowerupComponent.h"
#include "Components/PowerupCatcherComponent.h"
#include "Components/ShooterRocketComponent.h"
#include "Components/SpeedModifierComponent.h"
#include "Components/RocketSpawnComponent.h"
#include "Components/ShooterStateComponent.h"
#include "Components/AI/MoveToPointTaskComponent.h"
#include "Components/AI/AttackTaskComponent.h"
#include "Components/AI/ShooterBehaviorComponent.h"
#include "Components/AI/InvaderBehaviorComponent.h"
#include "Components/AI/WaitTaskComponent.h"
#include "Components/AI/CompositeTaskComponent.h"
#include "Components/AI/DodgeCenterTaskComponent.h"
#include "Components/AI/RandomMovementTaskComponent.h"
#include "Components/AI/SlideToBorderTaskComponent.h"
#include "Components/AI/WagToBorderTaskComponent.h"
#include "Components/AI/ShootIfSeeingTargetTaskComponent.h"
#include "Visibility/ObserverComponent.h"
#include "Visibility/ObservableComponent.h"
#include "Visibility/ObservableIdComponent.h"
#include "Visibility/CharacterVisibilityShapeComponent.h"
#include "Visibility/SimpleVisibilityShapeComponent.h"

#include "GameModes/Cubes/BigCubeComponent.h"
#include "GameModes/Cubes/SmallCubeComponent.h"

#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "Components/SingleComponents/EffectQueueSingleComponent.h"
#include "Components/SingleComponents/GameCollisionSingleComponent.h"
#include "Components/SingleComponents/GameModeSingleComponent.h"
#include "Components/SingleComponents/StatsLoggingSingleComponent.h"

#include "Systems/BotSystem.h"
#include "Systems/DamageSystem.h"
#include "Systems/ExplosionEffectSystem.h"
#include "Systems/GameInputSystem.h"
#include "Systems/GameModeSystem.h"
#include "Systems/GameModeSystemCars.h"
#include "Systems/GameShowSystem.h"
#include "Systems/GameStunningSystem.h"
#include "Systems/InvaderAttackSystem.h"
#include "Systems/InvaderConnectSystem.h"
#include "Systems/InvaderEntityFillSystem.h"
#include "Systems/InvaderMovingSystem.h"
#include "Systems/MarkerSystem.h"
#include "Systems/PhysicsProjectileInputSystem.h"
#include "Systems/PhysicsProjectileSystem.h"
#include "Systems/PlayerEntitySystem.h"
#include "Systems/PlayerStatsMonitoringSystem.h"
#include "Systems/PowerupSpawnSystem.h"
#include "Systems/PowerupSystem.h"
#include "Systems/ShooterCameraSystem.h"
#include "Systems/ShooterCarSystem.h"
#include "Systems/ShooterCarAttackSystem.h"
#include "Systems/ShooterAimSystem.h"
#include "Systems/ShooterCharacterAnimationSystem.h"
#include "Systems/ShooterEntityFillSystem.h"
#include "Systems/ShooterPlayerAttackSystem.h"
#include "Systems/ShooterPlayerConnectSystem.h"
#include "Systems/ShooterPlayerMovementSystem.h"
#include "Systems/ShooterProjectileSystem.h"
#include "Systems/ShooterRespawnSystem.h"
#include "Systems/ShootInputSystem.h"
#include "Systems/ShootSystem.h"
#include "Systems/StatsLoggingSystem.h"
#include "Systems/CreateGameModeSystem.h"
#include "Systems/ShooterRocketSystem.h"
#include "Systems/EnemyMovingSystem.h"
#include "Systems/DrawShootSystem.h"
#include "Visibility/ObservableIdSystem.h"
#include "Visibility/NetworkFrequencySystem.h"
#include "Visibility/ShooterVisibilitySystem.h"
#include "Visibility/SimpleVisibilitySystem.h"

#include "Systems/AI/ShooterBehaviorSystem.h"
#include "Systems/AI/InvaderBehaviorSystem.h"
#include "Systems/AI/BotTaskSystem.h"

#include "GameModes/Cubes/CubesEntityFillSystem.h"
#include "GameModes/Cubes/CubesGameplaySystem.h"
#include "GameModes/Cubes/CubesPlayerConnectSystem.h"

#include "Utils/StringUtils.h"

class RegisterReflectionForGamemode final
{
public:
    static void Cubes();
};

const DAVA::Vector<DAVA::String> GameMode::idNames = {
    "CARS",
    "TANKS",
    "SHOOTER",
    "INVADERS",
    "CUBES"
};

void RegisterGameComponents()
{
    /* PLEASE KEEP LOGICAL & ALPHABETICAL ORDER */

    using namespace DAVA;

    // Components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DamageComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ExplosionEffectComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameStunnableComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameStunningComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HealthComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PhysicsProjectileComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PlayerCarComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PlayerCharacterComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PlayerInvaderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PlayerTankComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PowerupCatcherComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PowerupComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RocketSpawnComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShootComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShootCooldownComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterAimComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterCarUserComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterProjectileComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterRocketComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterRoleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterStateComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedModifierComponent);

    // Bots related components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(AttackTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CompositeTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DodgeCenterTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InvaderBehaviorComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MoveToPointTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RandomMovementTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterBehaviorComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShootIfSeeingTargetTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SlideToBorderTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WagToBorderTaskComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaitTaskComponent);

    // Visibility components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ObserverComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ObservableComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ObservableIdComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CharacterVisibilityShapeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SimpleVisibilityShapeComponent);

    // Single components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BattleOptionsSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EffectQueueSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameCollisionSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameModeSingleComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StatsLoggingSingleComponent);

    // Systems
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CreateGameModeSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DamageSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DrawShootSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EnemyMovingSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ExplosionEffectSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameInputSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameModeSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameModeSystemCars);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameShowSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(GameStunningSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InvaderAttackSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InvaderConnectSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InvaderEntityFillSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InvaderMovingSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MarkerSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkFrequencySystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ObservableIdSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PhysicsProjectileInputSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PhysicsProjectileSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PlayerEntitySystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PlayerStatsMonitoringSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PowerupSpawnSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PowerupSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterAimSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterCameraSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterCarAttackSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterCarSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterCharacterAnimationSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterEntityFillSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterMovementSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterPlayerAttackSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterPlayerConnectSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterProjectileSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterRespawnSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterRocketSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterVisibilitySystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShootInputSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShootSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SimpleVisibilitySystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StatsLoggingSystem);

    // Bots related systems
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BotSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BotTaskSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(InvaderBehaviorSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ShooterBehaviorSystem);

    RegisterReflectionForGamemode::Cubes();
}

GameMode::Id GameMode::IdByName(DAVA::String name)
{
    std::transform(name.begin(), name.end(), name.begin(), toupper);
    DAVA::uint8 id = 0;
    for (const auto& idName : idNames)
    {
        if (idName == name)
        {
            return static_cast<Id>(id);
        }
        ++id;
    }

    return Id::TANKS;
};

void RegisterReflectionForGamemode::Cubes()
{
    // Components
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BigCubeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SmallCubeComponent);

    // Systems
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CubesEntityFillSystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CubesGameplaySystem);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CubesPlayerConnectSystem);
}