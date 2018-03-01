#pragma once

#include <Base/UnordererSet.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>

namespace DAVA
{
class Scene;
class UIControl;
}

class ShooterAimComponent;
class ShooterCarUserComponent;

// System that handles players interactions with cars
class ShooterCarSystem final : public DAVA::BaseSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCarSystem, DAVA::BaseSimulationSystem);

    ShooterCarSystem(DAVA::Scene* scene);

    void ProcessFixed(DAVA::float32 dt) override;

    void PrepareForRemove() override;

    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration);

    void SetInteractionControl(DAVA::UIControl* value);

private:
    struct CarInfo
    {
        DAVA::uint32 numPassengers;
        DAVA::uint32 firstFreeIndex;
    };

    // Get car the player sits in
    DAVA::Entity* GetParentCar(DAVA::Entity* player) const;

    // Get car the player is aiming at
    DAVA::Entity* GetTargetCar(ShooterAimComponent* aimComponent) const;

    // Return `true` if player is driving the car he is in
    bool IsDriver(DAVA::Entity* player) const;

    // Get information about the car
    CarInfo GetCarInfo(DAVA::Entity* car) const;

    // If all conditions are met, put a player into the car
    void TryPutInCar(ShooterCarUserComponent* playerCarUserComponent, DAVA::Entity* car);

    // Move plyaer out of the car he is in now
    void MoveOutOfCar(ShooterCarUserComponent* playerCarUserComponent);

    void UpdateInteractionControl(ShooterAimComponent* aimComponent);
    void ToggleCharacterStateIfRequired(DAVA::Entity* player) const;

private:
    DAVA::EntityGroup* entityGroup = nullptr;
    DAVA::UIControl* interactionControl;
    DAVA::UnorderedSet<ShooterAimComponent*> aimComponents;
};