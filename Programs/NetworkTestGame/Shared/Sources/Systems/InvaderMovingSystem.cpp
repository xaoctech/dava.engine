#include "InvaderMovingSystem.h"
#include "InputUtils.h"

#include "Components/HealthComponent.h"
#include "Components/PlayerInvaderComponent.h"
#include "Components/AI/InvaderBehaviorComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/ActionCollectSystem.h"

#include <Debug/ProfilerCPU.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>
#include <NetworkPhysics/NetworkPhysicsUtils.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <physx/PxQueryFiltering.h>
#include <physx/PxQueryReport.h>
#include <physx/PxRigidActor.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderSystem.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(InvaderMovingSystem)
{
    ReflectionRegistrator<InvaderMovingSystem>::Begin()[M::Tags("gm_invaders", "input")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &InvaderMovingSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 6.1f)]
    .End();
}

namespace InvaderMovingSystemDetail
{
static const Vector3 MOVE_SPEED(0.f, 30.f, 0.f);
static const Vector3 SLIDE_SPEED(30.f, 0.f, 0.f);
static const float32 ACCELERATION = 2.0f;

static const float32 TELEPORT_HALF_RANGE = 2000.f;

Vector2 GetNormalizedTeleportPosition(const Vector2& worldPosition)
{
    Vector2 normalizedPos;
    normalizedPos.x = Clamp(worldPosition.x / TELEPORT_HALF_RANGE, -1.f, 1.f);
    normalizedPos.y = Clamp(worldPosition.y / TELEPORT_HALF_RANGE, -1.f, 1.f);
    return normalizedPos;
}

inline Vector2 GetWorldTeleportPosition(const Vector2& normalizedPosition)
{
    return Vector2(normalizedPosition.x * TELEPORT_HALF_RANGE, normalizedPosition.y * TELEPORT_HALF_RANGE);
}
}

InvaderMovingSystem::InvaderMovingSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<PlayerInvaderComponent>())
{
    using namespace InvaderMovingSystemDetail;

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingleComponent<ActionsSingleComponent>();

    uint32 keyboardId = InputUtils::GetKeyboardDeviceId();
    actionsSingleComponent->CollectDigitalAction(UP, eInputElements::KB_W, keyboardId);
    actionsSingleComponent->CollectDigitalAction(DOWN, eInputElements::KB_S, keyboardId);
    actionsSingleComponent->CollectDigitalAction(LEFT, eInputElements::KB_A, keyboardId);
    actionsSingleComponent->CollectDigitalAction(RIGHT, eInputElements::KB_D, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ACCELERATE, eInputElements::KB_LSHIFT, keyboardId);

    actionsSingleComponent->AddAvailableAnalogAction(TELEPORT, AnalogPrecision::ANALOG_UINT16);

    entityGroup = scene->AquireEntityGroup<PlayerInvaderComponent>();
}

void InvaderMovingSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("InvaderMovingSystem::ProcessFixed");

    for (Entity* entity : entityGroup->GetEntities())
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
            ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, timeElapsed);
        }
    }
}

void InvaderMovingSystem::ApplyDigitalActions(Entity* entity, const Vector<FastName>& actions,
                                              uint32 clientFrameId, float32 duration)
{
    using namespace InvaderMovingSystemDetail;

    // apply movement actions
    Vector3 vec;

    for (const FastName& action : actions)
    {
        if (action == UP)
        {
            vec += MOVE_SPEED * duration;
        }
        else if (action == DOWN)
        {
            vec -= MOVE_SPEED * duration;
        }
        else if (action == LEFT)
        {
            vec -= SLIDE_SPEED * duration;
        }
        else if (action == RIGHT)
        {
            vec += SLIDE_SPEED * duration;
        }
        else if (action == ACCELERATE)
        {
            vec *= ACCELERATION;
        }
    }

    if (!vec.IsZero())
    {
        TransformComponent* transComp = entity->GetComponent<TransformComponent>();
        transComp->SetLocalTransform(transComp->GetPosition() + vec, transComp->GetRotation(), Vector3(1.0, 1.0, 1.0));
    }
}

void InvaderMovingSystem::ApplyAnalogActions(Entity* entity, const AnalogActionsMap& actions,
                                             uint32 clientFrameId, float32 duration)
{
    using namespace InvaderMovingSystemDetail;

    for (const auto& it : actions)
    {
        if (it.first.actionId == TELEPORT)
        {
            Vector2 analogPos = ConvertFixedPrecisionToAnalog(it.first.precision, it.second);
            Vector2 newPos2 = GetWorldTeleportPosition(analogPos);
            Vector3 newPos(newPos2.x, newPos2.y, 0.f);
            TransformComponent* transComp = entity->GetComponent<TransformComponent>();
            transComp->SetLocalTransform(newPos, transComp->GetRotation(), transComp->GetScale());
        }
    }
}
