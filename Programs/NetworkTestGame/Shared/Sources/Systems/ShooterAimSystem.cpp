#include "Systems/ShooterAimSystem.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "Components/SingleComponents/BattleOptionsSingleComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <Scene3D/Scene.h>
#include <Scene3D/Systems/ActionCollectSystem.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Entity/ComponentUtils.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Engine/Window.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <Input/TouchScreen.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>
#include <UI/UIControlBackground.h>
#include <UI/Joypad/UIJoypadComponent.h>
#include <Debug/ProfilerCPU.h>

#include <Physics/Private/PhysicsMath.h>
#include <Physics/PhysicsUtils.h>
#include <Physics/StaticBodyComponent.h>
#include <NetworkCore/Scene3D/Components/NetworkInputComponent.h>
#include <NetworkCore/NetworkCoreUtils.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterAimSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterAimSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterAimSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 12.0f)]
    .End();
}

ShooterAimSystem::ShooterAimSystem(DAVA::Scene* scene)
    : DAVA::INetworkInputSimulationSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterAimComponent>() | DAVA::ComponentUtils::MakeMask<DAVA::NetworkInputComponent>())
{
    using namespace DAVA;

    if (!IsServer(GetScene()))
    {
        Window* primaryWindow = GetPrimaryWindow();
        if (primaryWindow != nullptr)
        {
#if !defined(__DAVAENGINE_ANDROID__) && !defined(__DAVAENGINE_IOS__)
            if (GetEngineContext()->deviceManager->GetMouse() != nullptr)
            {
                primaryWindow->SetCursorCapture(eCursorCapture::PINNING);
            }
#endif
            primaryWindow->update.Connect(this, &ShooterAimSystem::OnUpdate);
        }

        BattleOptionsSingleComponent* battleOptionsSingleComponent = scene->GetSingletonComponent<BattleOptionsSingleComponent>();

        DVASSERT(battleOptionsSingleComponent->isSet);

        BattleControls& controls = battleOptionsSingleComponent->controls;
        currentAimUiControl = controls.currentAim;
        finalAimUiControl = controls.finalAim;
        movementJoypad = controls.movementJoypad;
    }

    actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();

    actionsSingleComponent->AddAvailableAnalogAction(SHOOTER_ACTION_ANALOG_ROTATE, AnalogPrecision::ANALOG_UINT16);
}

ShooterAimSystem::~ShooterAimSystem()
{
    DAVA::Engine::Instance()->update.Disconnect(this);
}

void ShooterAimSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    aimComponents.insert(aimComponent);
}

void ShooterAimSystem::RemoveEntity(DAVA::Entity* entity)
{
    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    aimComponents.erase(aimComponent);
}

void ShooterAimSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterAimSystem::ProcessFixed");

    ActionsSingleComponent* actionsSingleComponent = GetScene()->GetSingletonComponent<ActionsSingleComponent>();

    for (ShooterAimComponent* aimComponent : aimComponents)
    {
        DVASSERT(aimComponent != nullptr);

        Entity* aimingEntity = aimComponent->GetEntity();
        DVASSERT(aimingEntity != nullptr);

        // Process input
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), aimingEntity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();
            ApplyDigitalActions(aimingEntity, actions.digitalActions, actions.clientFrameId, dt);
            ApplyAnalogActions(aimingEntity, actions.analogActions, actions.clientFrameId, dt);
        }

        IncreaseCurrentAngles(aimingEntity);
        UpdateCurrentAimControlPosition(aimComponent);
    }
}

void ShooterAimSystem::Simulate(DAVA::Entity* entity)
{
    INetworkInputSimulationSystem::Simulate(entity);

    IncreaseCurrentAngles(entity);
}

void ShooterAimSystem::PrepareForRemove()
{
}

void ShooterAimSystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
}

void ShooterAimSystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
    using namespace DAVA;

    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();

    for (const auto& action : actions)
    {
        Vector2 analogActionState = ConvertFixedPrecisionToAnalog(action.first.precision, action.second);

        if (action.first.actionId == SHOOTER_ACTION_ANALOG_ROTATE)
        {
            ApplyDeviceIndependentAimRotation(aimComponent, analogActionState);
        }
    }
}

void ShooterAimSystem::OnUpdate(DAVA::Window*, DAVA::float32 elapsedTime)
{
    GenerateDeviceIndependentAimRotation();
}

void ShooterAimSystem::GenerateDeviceIndependentAimRotation() const
{
    using namespace DAVA;

    const EngineContext* engineContext = GetEngineContext();
    DVASSERT(engineContext != nullptr);

    DeviceManager* deviceManager = engineContext->deviceManager;
    DVASSERT(deviceManager != nullptr);

    Mouse* mouse = deviceManager->GetMouse();
    if (mouse != nullptr)
    {
        // Already delta since pinning is enabled
        AnalogElementState mouseDelta = mouse->GetPosition();

        float32 deltaX = mouseDelta.x;
        float32 deltaY = mouseDelta.y;
        if (deltaY != 0.0f || deltaX != 0.0f)
        {
            GenerateAimRotationFromDeltas(deltaX, deltaY);
        }
    }

    TouchScreen* touchScreen = deviceManager->GetTouchScreen();
    if (touchScreen != nullptr)
    {
        static uint32 touchId = UINT32_MAX;

        // If we're not rotating with touch, get touch id we should use
        if (touchId == UINT32_MAX)
        {
            if (movementJoypad->IsActive())
            {
                uint32 activeTouchId = movementJoypad->GetActiveTouchId() - 1;
                DVASSERT(activeTouchId < 10);

                touchId = (activeTouchId > 0) ? 0 : 1;
            }
            else
            {
                touchId = 0;
            }
        }

        // If it's inactive, skip
        DigitalElementState touchState = touchScreen->GetTouchStateByIndex(touchId);
        if (touchState.IsReleased())
        {
            touchId = UINT32_MAX;
        }

        if (touchId != UINT32_MAX)
        {
            static float32 lastX;
            static float32 lastY;

            if (touchState.IsJustPressed())
            {
                AnalogElementState touchPos = touchScreen->GetTouchPositionByIndex(touchId);
                lastX = touchPos.x;
                lastY = touchPos.y;
            }
            else if (touchState.IsPressed())
            {
                AnalogElementState touchPos = touchScreen->GetTouchPositionByIndex(touchId);

                float32 deltaX = touchPos.x - lastX;
                float32 deltaY = touchPos.y - lastY;
                if (deltaY != 0.0f || deltaX != 0.0f)
                {
                    GenerateAimRotationFromDeltas(deltaX, deltaY);
                }

                lastX = touchPos.x;
                lastY = touchPos.y;
            }
        }
    }
}

void ShooterAimSystem::GenerateAimRotationFromDeltas(DAVA::float32 deltaX, DAVA::float32 deltaY) const
{
    using namespace DAVA;

    static const float32 RADIANS_PER_SCREEN = PI * 1.0f;

    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
    Size2i windowSize = vcs->GetVirtualScreenSize();

    // Normalized in screen space
    deltaX /= windowSize.dx;
    deltaY /= windowSize.dy;

    // In radians
    deltaX *= RADIANS_PER_SCREEN;
    deltaY *= RADIANS_PER_SCREEN;

    deltaX = Clamp(deltaX, -1.0f, 1.0f);
    deltaY = Clamp(deltaY, -1.0f, 1.0f);

    for (ShooterAimComponent* aimComponent : aimComponents)
    {
        if (IsClientOwner(GetScene(), aimComponent->GetEntity()))
        {
            AddAnalogActionForClient(GetScene(), aimComponent->GetEntity(), SHOOTER_ACTION_ANALOG_ROTATE, Vector2(deltaX, deltaY));
        }
    }
}

void ShooterAimSystem::ApplyDeviceIndependentAimRotation(ShooterAimComponent* aimComponent, DAVA::Vector2 analogActionState) const
{
    aimComponent->SetFinalAngleX(aimComponent->GetFinalAngleX() + analogActionState.y);
    aimComponent->SetFinalAngleZ(aimComponent->GetFinalAngleZ() + analogActionState.x);
}

void ShooterAimSystem::IncreaseCurrentAngles(DAVA::Entity* aimingEntity)
{
    using namespace DAVA;

    DVASSERT(aimingEntity != nullptr);

    ShooterAimComponent* aimComponent = aimingEntity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    aimComponent->SetCurrentAngleZ(Lerp(aimComponent->GetCurrentAngleZ(), aimComponent->GetFinalAngleZ(), SHOOTER_CHARACTER_ROTATION_SPEED));
    aimComponent->SetCurrentAngleX(Lerp(aimComponent->GetCurrentAngleX(), aimComponent->GetFinalAngleX(), SHOOTER_CHARACTER_ROTATION_SPEED));
}

void ShooterAimSystem::UpdateCurrentAimControlPosition(ShooterAimComponent* aimComponent)
{
    using namespace DAVA;

    DVASSERT(aimComponent != nullptr);

    if (currentAimUiControl == nullptr || finalAimUiControl == nullptr)
    {
        return;
    }

    Entity* aimingEntity = aimComponent->GetEntity();
    DVASSERT(aimingEntity != nullptr);

    ShooterCarUserComponent* carUserComponent = aimingEntity->GetComponent<ShooterCarUserComponent>();
    DVASSERT(carUserComponent != nullptr);

    if (carUserComponent->GetCarNetworkId() != NetworkID::INVALID)
    {
        currentAimUiControl->SetVisibilityFlag(false);
        finalAimUiControl->SetVisibilityFlag(false);
        return;
    }
    else
    {
        currentAimUiControl->SetVisibilityFlag(true);
        finalAimUiControl->SetVisibilityFlag(true);
    }

    // Get current aim ray to raycast through it
    Vector3 aimRayOrigin;
    Vector3 aimRayDirection;
    Vector3 aimRayEnd;
    Entity* aimEndEntity;
    GetCurrentAimRay(*aimComponent, RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_DYNAMICS, aimRayOrigin, aimRayDirection, aimRayEnd, &aimEndEntity);

    auto shootStartEntity = aimComponent->GetEntity()->FindByName(SHOOTER_GUN_BARREL_ENTITY_NAME);
    if (shootStartEntity != nullptr)
    {
        Vector3 shootStart = shootStartEntity->GetComponent<TransformComponent>()->GetWorldTransform().GetTranslationVector();
        Vector3 shootVector = aimRayEnd - shootStart;
        float32 shootVectorLength = shootVector.Length();
        shootVector.Normalize();

        // Check if we collide with anything when shooting from the gun towards the end point
        // If we do, draw aim at that point, otherwise draw it at the end of the ray
        physx::PxRaycastHit hit;
        QueryFilterCallback filterCallback(aimComponent->GetEntity(), RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_DYNAMICS);
        bool collision = GetRaycastHit(*GetScene(), shootStart, shootVector, shootVectorLength, &filterCallback, hit);
        if (collision)
        {
            Component* component = static_cast<Component*>(hit.actor->userData);
            DVASSERT(component != nullptr);

            if (component->GetType()->Is<StaticBodyComponent>())
            {
                aimRayEnd = PhysicsMath::PxVec3ToVector3(hit.position);
            }
        }

        // Update UI control's position

        Camera* camera = GetScene()->GetCamera(0);
        DVASSERT(camera != nullptr);

        VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
        Size2i windowSize = vcs->GetVirtualScreenSize();
        Vector2 currentAimScreenPosition = camera->GetOnScreenPosition(aimRayEnd, Rect(0.0f, 0.0f, static_cast<float32>(windowSize.dx), static_cast<float32>(windowSize.dy)));
        currentAimUiControl->SetPosition(currentAimScreenPosition);
    }
}
