#include "Systems/ShooterCameraSystem.h"
#include "Components/ShooterAimComponent.h"
#include "Components/ShooterCarUserComponent.h"
#include "ShooterUtils.h"
#include "ShooterConstants.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/UIControlSystem.h>
#include <Debug/ProfilerCPU.h>

#include <NetworkCore/NetworkCoreUtils.h>

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterCameraSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterCameraSystem>::Begin()[M::Tags("gm_shooter", "client")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterCameraSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 2.1f)] // We do not have a group after transform and between renderer, so use engine end for now
    .End();
}

ShooterCameraSystem::ShooterCameraSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, DAVA::ComponentUtils::MakeMask<ShooterAimComponent>())
    , cameraComponent(new DAVA::CameraComponent())
    , trackedAimComponent(nullptr)
{
    using namespace DAVA;

    // Create a camera and an entity for it

    const EngineContext* engineContext = GetEngineContext();
    DVASSERT(engineContext != nullptr);

    UIControlSystem* uiControlSystem = engineContext->uiControlSystem;
    DVASSERT(uiControlSystem != nullptr);

    VirtualCoordinatesSystem* vcs = uiControlSystem->vcs;
    DVASSERT(vcs != nullptr);

    Size2i physicalWindowSize = vcs->GetPhysicalScreenSize();
    float32 screenAspectRatio = static_cast<float32>(physicalWindowSize.dx) / static_cast<float32>(physicalWindowSize.dy);

    ScopedPtr<Camera> camera(GetScene()->GetCamera(0));
    camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
    camera->SetPosition(Vector3(0.0f, 50.0f, 50.0f));
    camera->SetTarget(Vector3(0.0f, 0.0f, 0.0f));
    camera->RebuildCameraFromValues();
    camera->SetupPerspective(70.f, screenAspectRatio, 0.001f, 1000.0f);
    GetScene()->SetCurrentCamera(camera);

    cameraComponent->SetCamera(camera);

    Entity* cameraEntity = new Entity();
    cameraEntity->SetName("AimCamera");
    cameraEntity->AddComponent(cameraComponent);

    scene->AddNode(cameraEntity);
}

void ShooterCameraSystem::AddEntity(DAVA::Entity* entity)
{
    ShooterAimComponent* aimComponent = entity->GetComponent<ShooterAimComponent>();
    DVASSERT(aimComponent != nullptr);

    if (IsClientOwner(GetScene(), entity))
    {
        DVASSERT(trackedAimComponent == nullptr);
        trackedAimComponent = aimComponent;
    }
}

void ShooterCameraSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (trackedAimComponent != nullptr && entity == trackedAimComponent->GetEntity())
    {
        trackedAimComponent = nullptr;
    }
}

void ShooterCameraSystem::Process(DAVA::float32 dt)
{
    using namespace DAVA;

    DAVA_PROFILER_CPU_SCOPE("ShooterCameraSystem::ProcessFixed");

    if (trackedAimComponent != nullptr)
    {
        Entity* aimingEntity = trackedAimComponent->GetEntity();
        DVASSERT(aimingEntity != nullptr);

        ShooterCarUserComponent* carUserComponent = aimingEntity->GetComponent<ShooterCarUserComponent>();
        DVASSERT(carUserComponent != nullptr);

        // If we're in the car, still use aim so that when player gets out he is rotated towards camera direction
        if (carUserComponent->GetCarNetworkId() != NetworkID::INVALID)
        {
            Entity* car = GetEntityWithNetworkId(GetScene(), carUserComponent->GetCarNetworkId());
            DVASSERT(car != nullptr);

            Matrix4 rotationMatrixZ;
            rotationMatrixZ.BuildRotation(Vector3::UnitZ, trackedAimComponent->GetFinalAngleZ());

            TransformComponent* focusedCarTransform = car->GetComponent<TransformComponent>();
            DVASSERT(focusedCarTransform != nullptr);

            Vector3 finalPosition = focusedCarTransform->GetPosition() + SHOOTER_CAR_CAMERA_OFFSET * rotationMatrixZ;
            Vector3 finalTarget = focusedCarTransform->GetPosition();

            Camera* camera = cameraComponent->GetCamera();
            camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            camera->SetPosition(finalPosition);
            camera->SetTarget(finalTarget);
            camera->RebuildCameraFromValues();
        }
        else
        {
            // Get aim ray to align camera. Camera always looks towards the final aim
            Vector3 aimRayOrigin;
            Vector3 aimRayDirection;
            Vector3 aimRayEnd;
            Entity* aimRayEndEntity;
            GetFinalAimRay(*trackedAimComponent, RaycastFilter::IGNORE_SOURCE | RaycastFilter::IGNORE_DYNAMICS, aimRayOrigin, aimRayDirection, aimRayEnd, &aimRayEndEntity);

            // Update camera properties
            Camera* camera = cameraComponent->GetCamera();
            camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            camera->SetPosition(aimRayOrigin);
            camera->SetTarget(aimRayEnd);
            camera->RebuildCameraFromValues();
        }
    }
}

void ShooterCameraSystem::PrepareForRemove()
{
}
