#pragma once

#include <Base/UnordererSet.h>

#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

namespace DAVA
{
class Scene;
class UIControl;
}

class ShooterAimComponent;

// System that handles players interactions with cars
class ShooterCarSystem final : public DAVA::INetworkInputSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterCarSystem, DAVA::INetworkInputSimulationSystem);

    ShooterCarSystem(DAVA::Scene* scene);
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 dt) override;
    void PrepareForRemove() override;
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;

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

    void UpdateInteractionControl(ShooterAimComponent* aimComponent);
    void ToggleCharacterStateIfRequired(DAVA::Entity* player) const;

private:
    DAVA::UIControl* interactionControl;
    DAVA::UnorderedSet<ShooterAimComponent*> aimComponents;
};