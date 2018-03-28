#include "GameModes/Cubes/CubesGameplaySystem.h"

#include "GameModes/Cubes/CubesUtils.h"

#include "GameModes/Cubes/BigCubeComponent.h"
#include "GameModes/Cubes/SmallCubeComponent.h"

#include "InputUtils.h"

#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkResimulationSingleComponent.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/Core/BoxShapeComponent.h>
#include <Physics/Core/PlaneShapeComponent.h>
#include <Physics/Core/StaticBodyComponent.h>
#include <Physics/Core/Private/PhysicsMath.h>

#include <Engine/EngineContext.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Scene3D/Components/TransformComponent.h>

DAVA_VIRTUAL_REFLECTION_IMPL(CubesGameplaySystem)
{
    using namespace DAVA;
    ReflectionRegistrator<CubesGameplaySystem>::Begin()[M::Tags("gm_cubes")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &CubesGameplaySystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 25.3f)]
    .End();
}

namespace CubesGameplaySystemDetails
{
using namespace DAVA;

static const Vector3 BigCubeCameraOffset = { 0.0f, 30.0f, 10.0f };
static const float32 BigCubeImpulseStep = 1.0f;
static const float32 MinForceDistance = 4.0f;
} // namespace CubesGameplaySystemDetails

CubesGameplaySystem::CubesGameplaySystem(DAVA::Scene* scene)
    : DAVA::BaseSimulationSystem(scene, DAVA::ComponentMask())
    , bigCubes(scene->AquireEntityGroup<BigCubeComponent>())
    , smallCubes(scene->AquireEntityGroup<SmallCubeComponent>())
{
    using namespace DAVA;

    for (Entity* bigCube : bigCubes->GetEntities())
    {
        if (IsClientOwner(bigCube))
        {
            DVASSERT(localCube == nullptr);
            localCube = bigCube;
        }
    }

    bigCubes->onEntityAdded->Connect(this, [this](Entity* e) {
        if (IsClientOwner(e))
        {
            DVASSERT(localCube == nullptr);
            localCube = e;
        }
    });

    bigCubes->onEntityRemoved->Connect(this, [this](Entity* e) {
        if (IsClientOwner(e) && localCube != nullptr)
        {
            DVASSERT(localCube == e);
            localCube = nullptr;
        }
    });

    uint32 keyboardId = InputUtils::GetKeyboardDeviceId();

    using namespace CubesDetails;

    NetworkResimulationSingleComponent* networkResimulationSingleComponent = scene->GetSingleComponent<NetworkResimulationSingleComponent>();
    networkResimulationSingleComponent->boundingBoxResimulation.enabled = true;

    networkResimulationSingleComponent->boundingBoxResimulation.SetBuilder([this](Entity* e, float32 inflation) {
        AABBox3 box;

        DynamicBodyComponent* dynamicBody = e->GetComponent<DynamicBodyComponent>();

        DVASSERT(dynamicBody != nullptr);

        physx::PxBounds3 physicsBox = dynamicBody->GetPxActor()->getWorldBounds(1.f);

        if (e->GetComponent<BigCubeComponent>() != nullptr)
        {
            physicsBox.fattenSafe(CubesGameplaySystemDetails::MinForceDistance - CubesDetails::BigCubeHalfSize);
        }
        else if (e->GetComponent<SmallCubeComponent>() == nullptr)
        {
            DVASSERT(false);
        }

        physicsBox.scaleSafe(inflation);

        box = PhysicsMath::PxBounds3ToAABox3(physicsBox);

        return box;
    });

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingleComponent<ActionsSingleComponent>();
    actionsSingleComponent->CollectDigitalAction(ActionForward, eInputElements::KB_W, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ActionBack, eInputElements::KB_S, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ActionLeft, eInputElements::KB_A, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ActionRight, eInputElements::KB_D, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ActionUp, eInputElements::KB_SPACE, keyboardId);
    actionsSingleComponent->CollectDigitalAction(ActionFastUp, eInputElements::KB_LALT, keyboardId, DigitalElementState::JustPressed());

    // Joypad.
    actionsSingleComponent->AddAvailableAnalogAction(FastName("LMOVE"), AnalogPrecision::ANALOG_UINT8);

    CubesUtils::SetupCubesScene(scene); // TODO: add some handy way for scene creation.
}

CubesGameplaySystem::~CubesGameplaySystem()
{
    bigCubes->onEntityAdded->Disconnect(this);
    bigCubes->onEntityRemoved->Disconnect(this);
}

void CubesGameplaySystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    /**
        Gamemode is not tested on multiple clients, but modifications and fixes should be trivial.
    */

    for (Entity* bigCube : bigCubes->GetEntities())
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), bigCube);
        if (!allActions.empty())
        {
            const auto& actions = allActions.front();
            ApplyDigitalActions(bigCube, actions.digitalActions, actions.clientFrameId, timeElapsed);
            ApplyAnalogActions(bigCube, actions.analogActions, actions.clientFrameId, timeElapsed);
        }

        for (Entity* smallCube : smallCubes->GetEntities())
        {
            ScheduleMagneteForces(bigCube, smallCube);
        }
    }

    if (IsClient(this) && !IsReSimulating())
    {
        UpdateCameraPosition(localCube);
    }
}

void CubesGameplaySystem::ApplyAnalogActions(DAVA::Entity* entity, const DAVA::AnalogActionsMap& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration)
{
    using namespace DAVA;
    using CubesGameplaySystemDetails::BigCubeImpulseStep;

    Vector3 impulse = Vector3::Zero;

    for (const auto& action : actions)
    {
        if (action.first.actionId == FastName("LMOVE"))
        {
            Vector2 analogPos = ConvertFixedPrecisionToAnalog(action.first.precision, action.second);
            if (!FLOAT_EQUAL(analogPos.x, 0.0f))
            {
                // TODO: fix this inverted logic on all gamemodes someday.
                impulse.x += analogPos.x < 0.f ? BigCubeImpulseStep : -BigCubeImpulseStep;
            }
            if (!FLOAT_EQUAL(analogPos.y, 0.0f))
            {
                impulse.y += analogPos.y > 0.f ? BigCubeImpulseStep : -BigCubeImpulseStep;
            }
        }
    }

    if (!impulse.IsZero())
    {
        GetScene()->GetSystem<PhysicsSystem>()->AddForce(entity->GetComponent<DynamicBodyComponent>(), impulse, physx::PxForceMode::eIMPULSE);
    }
}

void CubesGameplaySystem::ApplyDigitalActions(DAVA::Entity* entity, const DAVA::Vector<DAVA::FastName>& actions, DAVA::uint32 clientFrameId, DAVA::float32 duration) const
{
    using namespace DAVA;
    using CubesGameplaySystemDetails::BigCubeImpulseStep;

    Vector3 impulse = Vector3::Zero;

    for (const FastName& action : actions)
    {
        if (action == CubesDetails::ActionForward)
        {
            impulse.y -= BigCubeImpulseStep;
        }
        else if (action == CubesDetails::ActionBack)
        {
            impulse.y += BigCubeImpulseStep;
        }
        else if (action == CubesDetails::ActionLeft)
        {
            impulse.x += BigCubeImpulseStep;
        }
        else if (action == CubesDetails::ActionRight)
        {
            impulse.x -= BigCubeImpulseStep;
        }
        else if (action == CubesDetails::ActionUp)
        {
            impulse.z += BigCubeImpulseStep;
        }
        else if (action == CubesDetails::ActionFastUp)
        {
            impulse.z += BigCubeImpulseStep * 15.f;
        }
    }

    if (!impulse.IsZero())
    {
        GetScene()->GetSystem<PhysicsSystem>()->AddForce(entity->GetComponent<DynamicBodyComponent>(), impulse, physx::PxForceMode::eIMPULSE);
    }
}

void CubesGameplaySystem::ScheduleMagneteForces(DAVA::Entity* bigCube, DAVA::Entity* smallCube) const
{
    using namespace DAVA;

    const Vector3 smallCubePosition = smallCube->GetComponent<TransformComponent>()->GetPosition();
    const Vector3 bigCubePosition = bigCube->GetComponent<TransformComponent>()->GetPosition();
    const float32 distanceToBigCube = Distance(bigCubePosition, smallCubePosition);
    if (distanceToBigCube < CubesGameplaySystemDetails::MinForceDistance)
    {
        const Vector3 forceDirection = Normalize(bigCubePosition - smallCubePosition);
        GetScene()->GetSystem<PhysicsSystem>()->AddForce(smallCube->GetComponent<DynamicBodyComponent>(), forceDirection / distanceToBigCube * 3.0f, physx::PxForceMode::eFORCE);
    }
}

void CubesGameplaySystem::UpdateCameraPosition(DAVA::Entity* bigCube) const
{
    using namespace DAVA;

    if (bigCube != nullptr)
    {
        TransformComponent* cubeTransform = bigCube->GetComponent<TransformComponent>();
        Vector3 cubePosition = cubeTransform->GetPosition();

        Camera* camera = GetScene()->GetCurrentCamera();

        camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        camera->SetPosition(cubePosition + CubesGameplaySystemDetails::BigCubeCameraOffset);
        camera->SetTarget(cubePosition);
        camera->RebuildCameraFromValues();
    }
}
