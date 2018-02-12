#pragma once

#include <Base/FastName.h>
#include <Base/UnordererSet.h>
#include <Entity/SceneSystem.h>

#include <NetworkCore/Scene3D/Systems/INetworkInputSimulationSystem.h>

namespace DAVA
{
class Window;
class Scene;
class UIControl;
class UIJoypadComponent;
class ActionsSingleComponent;
}

class ShooterAimComponent;

// System that manages aims and according UI
class ShooterAimSystem final : public DAVA::INetworkInputSimulationSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(ShooterAimSystem, DAVA::INetworkInputSimulationSystem);

    ShooterAimSystem(DAVA::Scene* scene);
    ~ShooterAimSystem();
    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void ProcessFixed(DAVA::float32 dt) override;
    void Simulate(DAVA::Entity* entity) override;
    void PrepareForRemove() override;
    void ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;
    void ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const override;

    void OnUpdate(DAVA::Window*, DAVA::float32 elapsedTime);

    void GenerateDeviceIndependentAimRotation() const;
    void GenerateAimRotationFromDeltas(DAVA::float32 deltaX, DAVA::float32 deltaY) const;
    void ApplyDeviceIndependentAimRotation(ShooterAimComponent* aimComponent, DAVA::Vector2 analogActionState) const;
    void IncreaseCurrentAngles(DAVA::Entity* aimingEntity);
    void UpdateCurrentAimControlPosition(ShooterAimComponent* aimComponent);

private:
    DAVA::ActionsSingleComponent* actionsSingleComponent = nullptr;
    DAVA::UnorderedSet<ShooterAimComponent*> aimComponents;
    DAVA::UIControl* currentAimUiControl = nullptr;
    DAVA::UIControl* finalAimUiControl = nullptr;
    DAVA::UIJoypadComponent* movementJoypad = nullptr; // TODO: Only to check if we're touching joypad, to block rotation. Better be done with UI changes, but it's the simpler way for now
};
