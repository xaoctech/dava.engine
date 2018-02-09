#include "GameInputSystem.h"

#include "Base/FastName.h"
#include "Engine/Engine.h"
#include <Debug/ProfilerCPU.h>
#include "Scene3D/Scene.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/ActionCollectSystem.h"
#include "Systems/GameModeSystem.h"
#include "Systems/GameModeSystemCars.h"
#include "Systems/GameModeSystemCharacters.h"
#include "Time/SystemTimer.h"

#include "NetworkCore/Scene3D/Components/NetworkInputComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPredictComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Systems/NetworkInputSystem.h"
#include "NetworkCore/Scene3D/Systems/NetworkTimeSystem.h"
#include "NetworkCore/NetworkCoreUtils.h"

#include "Components/GameStunnableComponent.h"
#include "Components/HealthComponent.h"
#include "Components/SpeedModifierComponent.h"

#include <Physics/VehicleCarComponent.h>
#include <Physics/PhysicsUtils.h>
#include <Physics/CharacterControllerComponent.h>

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(GameInputSystem)
{
    ReflectionRegistrator<GameInputSystem>::Begin()[M::Tags("gameinput")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &GameInputSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 5.0f)]
    .End();
}

namespace GameInputSystemDetail
{
static const FastName UP("UP");
static const FastName DOWN("DOWN");
static const FastName LEFT("LEFT");
static const FastName RIGHT("RIGHT");

static const FastName CAM_UP("CAM_UP");
static const FastName CAM_DOWN("CAM_DOWN");
static const FastName CAM_FWD("CAM_FWD");
static const FastName CAM_BKWD("CAM_BKWD");
static const FastName CAM_LEFT("CAM_LEFT");
static const FastName CAM_RIGHT("CAM_RIGHT");

static const FastName LMOVE("LMOVE");
static const FastName RMOVE("RMOVE");

static const FastName TELEPORT("TELEPORT");

static const Vector3 MOV_SPEED(0.f, 10.f, 0.f);
static const float32 ROT_SPEED = 60.f * DEG_TO_RAD;
static const float32 TELEPORT_HALF_RANGE = 2000.f;

#ifndef SERVER
static const float32 CAM_MOVE_SPEED_MIN = 100.f;
static const float32 CAM_MOVE_SPEED_MAX = 500.f;
static const float32 CAM_MOVE_SPEED_FACTOR = 1.f;
#endif

template <typename T>
String GetName(const T&)
{
    return "Unknown";
}

template <>
String GetName(const Matrix4&)
{
    return "Matrix4";
}

template <>
String GetName(const Vector3&)
{
    return "Vector3";
}

template <>
String GetName(const Quaternion&)
{
    return "Quaternion";
}

template <typename T>
bool CompareTransform(const T& lhs, const T& rhs, uint32 size, float32 epsilon, uint32 frameId)
{
    for (uint32 i = 0; i < size; ++i)
    {
        if (!FLOAT_EQUAL_EPS(lhs.data[i], rhs.data[i], epsilon))
        {
            Logger::Debug("Transforms aren't equal, diff: %f, index: %d, type: %s, frame: %d", std::abs(lhs.data[i] - rhs.data[i]), i, GetName(lhs).c_str(), frameId);

            return false;
        }
    }
    return true;
}

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

GameInputSystem::GameInputSystem(Scene* scene)
    : INetworkInputSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>() | ComponentUtils::MakeMask<NetworkInputComponent>())
{
    using namespace GameInputSystemDetail;

    uint32 keyboardId = GetKeyboardDeviceId();

    ActionsSingleComponent* actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
    actionsSingleComponent->CollectDigitalAction(UP, eInputElements::KB_W, keyboardId);
    actionsSingleComponent->CollectDigitalAction(DOWN, eInputElements::KB_S, keyboardId);
    actionsSingleComponent->CollectDigitalAction(LEFT, eInputElements::KB_A, keyboardId);
    actionsSingleComponent->CollectDigitalAction(RIGHT, eInputElements::KB_D, keyboardId);

    actionsSingleComponent->CollectDigitalAction(CAM_FWD, eInputElements::KB_UP, keyboardId);
    actionsSingleComponent->CollectDigitalAction(CAM_BKWD, eInputElements::KB_DOWN, keyboardId);
    actionsSingleComponent->CollectDigitalAction(CAM_LEFT, eInputElements::KB_LEFT, keyboardId);
    actionsSingleComponent->CollectDigitalAction(CAM_RIGHT, eInputElements::KB_RIGHT, keyboardId);
    actionsSingleComponent->CollectDigitalAction(CAM_UP, eInputElements::KB_PAGEUP, keyboardId);
    actionsSingleComponent->CollectDigitalAction(CAM_DOWN, eInputElements::KB_PAGEDOWN, keyboardId);

    actionsSingleComponent->AddAvailableAnalogAction(LMOVE, AnalogPrecision::ANALOG_UINT8);
    actionsSingleComponent->AddAvailableAnalogAction(RMOVE, AnalogPrecision::ANALOG_UINT8);
    actionsSingleComponent->AddAvailableAnalogAction(TELEPORT, AnalogPrecision::ANALOG_UINT16);
}

void GameInputSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("GameInputSystem::ProcessFixed");

    for (Entity* entity : entities)
    {
        const Vector<ActionsSingleComponent::Actions>& allActions = GetCollectedActionsForClient(GetScene(), entity);
        if (!allActions.empty())
        {
            const auto& actions = allActions.back();
            ApplyDigitalActions(entity, actions.digitalActions, actions.clientFrameId, timeElapsed);
            ApplyAnalogActions(entity, actions.analogActions, actions.clientFrameId, timeElapsed);
#ifdef SERVER
            ApplyCameraDelta(entity, actions.cameraDelta, actions.clientFrameId, timeElapsed);
#endif
        }
    }
}

void GameInputSystem::ApplyDigitalActions(Entity* entity,
                                          const Vector<FastName>& actions,
                                          uint32 clientFrameId,
                                          DAVA::float32 duration) const
{
    if (!CanMove(entity))
    {
        return;
    }

    using namespace GameInputSystemDetail;

    TransformComponent* transComp = entity->GetComponent<TransformComponent>();

    if (GetScene()->GetSystem<GameModeSystemCars>() != nullptr)
    {
        VehicleCarComponent* car = entity->GetComponent<VehicleCarComponent>();
        if (car != nullptr)
        {
            float32 acceleration = 0.0f;
            float32 steer = 0.0f;

            for (const FastName& action : actions)
            {
                if (action == UP)
                {
                    acceleration += 1.0f;
                }
                else if (action == DOWN)
                {
                    acceleration -= 1.0f;
                }
                else if (action == LEFT)
                {
                    steer += 1.0f;
                }
                else if (action == RIGHT)
                {
                    steer -= 1.0f;
                }
            }

            car->SetAnalogSteer(Clamp(steer, -1.0f, 1.0f));

            if (acceleration < 0.0f)
            {
                car->SetGear(eVehicleGears::Reverse);
            }
            else if (acceleration != 0.0f)
            {
                car->SetGear(eVehicleGears::First);
            }

            car->SetAnalogAcceleration(std::abs(Clamp(acceleration, -1.0f, 1.0f)));
        }
    }
    else if (GetScene()->GetSystem<GameModeSystemCharacters>() != nullptr)
    {
        CharacterControllerComponent* controller = PhysicsUtils::GetCharacterControllerComponent(entity);

        if (controller != nullptr)
        {
            Vector3 vec;
            float32 angle = 0.f;

            for (const FastName& action : actions)
            {
                if (action == UP)
                {
                    vec += MOV_SPEED * duration;
                }
                else if (action == DOWN)
                {
                    vec -= MOV_SPEED * duration;
                }
                else if (action == LEFT)
                {
                    angle -= ROT_SPEED * duration;
                }
                else if (action == RIGHT)
                {
                    angle += ROT_SPEED * duration;
                }
            }

            Quaternion rotation = transComp->GetRotation();
            Vector3 position = transComp->GetPosition();
            rotation *= Quaternion::MakeRotation(Vector3::UnitZ, -angle);
            transComp->SetLocalTransform(position, rotation, Vector3(1.0, 1.0, 1.0));

            Vector3 displacement = rotation.ApplyToVectorFast(vec);
            controller->Move(displacement);
        }
    }
    else
    {
        SpeedModifierComponent* speedModifier = entity->GetComponent<SpeedModifierComponent>();
        float32 speedFactor = speedModifier ? speedModifier->GetFactor() : 1.f;

        Vector3 vec;
        float32 angle = 0.f;

        for (const FastName& action : actions)
        {
            if (action == UP)
            {
                //#ifndef SERVER
                //            static uint32 index = 0;
                //            if (++index % 200 == 0)
                //            {
                //                duration *= 100;
                //            }
                //#endif
                vec += MOV_SPEED * speedFactor * duration;
            }
            else if (action == DOWN)
            {
                vec -= MOV_SPEED * speedFactor * duration;
            }
            else if (action == LEFT)
            {
                angle -= ROT_SPEED * duration;
            }
            else if (action == RIGHT)
            {
                angle += ROT_SPEED * duration;
            }
        }

        if (!vec.IsZero() || angle != 0.0f)
        {
            Quaternion rotation = transComp->GetRotation();
            Vector3 position = transComp->GetPosition();
            rotation *= Quaternion::MakeRotation(Vector3::UnitZ, -angle);
            position += rotation.ApplyToVectorFast(vec);
            transComp->SetLocalTransform(position, rotation, Vector3(1.0, 1.0, 1.0));
        }
    }
    
#ifndef SERVER
    CameraComponent* cameraComp = entity->GetComponent<CameraComponent>();
    if (nullptr != cameraComp)
    {
        Camera* camera = cameraComp->GetCamera();
        Vector3 curCamPos = camera->GetPosition();
        float curCamMoveSpeed = Clamp(curCamPos.z * CAM_MOVE_SPEED_FACTOR, CAM_MOVE_SPEED_MIN, CAM_MOVE_SPEED_MAX);
        Vector3 offset;
        for (const FastName& action : actions)
        {
            if (action == CAM_UP)
            {
                offset.z += curCamMoveSpeed * duration;
            }
            if (action == CAM_DOWN)
            {
                offset.z -= curCamMoveSpeed * duration;
            }
            if (action == CAM_FWD)
            {
                offset.y += curCamMoveSpeed * duration;
            }
            if (action == CAM_BKWD)
            {
                offset.y -= curCamMoveSpeed * duration;
            }
            if (action == CAM_RIGHT)
            {
                offset.x += curCamMoveSpeed * duration;
            }
            if (action == CAM_LEFT)
            {
                offset.x -= curCamMoveSpeed * duration;
            }
        }

        camera->SetPosition(camera->GetPosition() + offset);
        camera->SetTarget(camera->GetTarget() + offset);
        // TODO: reconsider
        camera->Rotate(Quaternion());
    }
#endif
}

void GameInputSystem::ApplyAnalogActions(Entity* entity,
                                         const AnalogActionsMap& actions,
                                         uint32 clientFrameId,
                                         float32 duration) const
{
    if (!CanMove(entity))
    {
        return;
    }

    using namespace GameInputSystemDetail;
    for (const auto& action : actions)
    {
        if (action.first.actionId == LMOVE)
        {
            Vector2 analogPos = ConvertFixedPrecisionToAnalog(action.first.precision, action.second);
            if (GetScene()->GetSystem<GameModeSystemCars>() != nullptr)
            {
                if (!FLOAT_EQUAL(analogPos.x, 0.0f) && !FLOAT_EQUAL(analogPos.y, 0.0f))
                {
                    VehicleCarComponent* car = entity->GetComponent<VehicleCarComponent>();

                    if (car != nullptr)
                    {
                        car->SetAnalogAcceleration(std::abs(analogPos.y));
                        if (analogPos.y > 0)
                        {
                            car->SetGear(eVehicleGears::Reverse);
                        }
                        else
                        {
                            car->SetGear(eVehicleGears::First);
                        }

                        car->SetAnalogSteer(-analogPos.x);
                    }
                }
            }
            else
            {
                Vector3 vec = -MOV_SPEED * duration * analogPos.y;
                float32 angle = ROT_SPEED * duration * analogPos.x;

                TransformComponent* transComp = entity->GetComponent<TransformComponent>();
                Quaternion rotation = transComp->GetRotation();
                Vector3 position = transComp->GetPosition();
                rotation *= Quaternion::MakeRotation(Vector3::UnitZ, -angle);
                position += rotation.ApplyToVectorFast(vec);
                transComp->SetLocalTransform(position, rotation, Vector3(1.0, 1.0, 1.0));
            }
        }
        else if (action.first.actionId == TELEPORT)
        {
            if (GetScene()->GetSystem<GameModeSystem>() != nullptr)
            {
                Vector2 analogPos = ConvertFixedPrecisionToAnalog(action.first.precision, action.second);
                Vector2 newPos2 = GetWorldTeleportPosition(analogPos);
                Vector3 newPos(newPos2.x, newPos2.y, 0.f);
                TransformComponent* transComp = entity->GetComponent<TransformComponent>();
                transComp->SetLocalTransform(newPos, transComp->GetRotation(), transComp->GetScale());
            }
        }
    }
}

void GameInputSystem::ApplyCameraDelta(DAVA::Entity* entity, const Quaternion& cameraDelta,
                                       uint32 clientFrameId,
                                       DAVA::float32 duration) const
{
    CameraComponent* cameraComp = entity->GetComponent<CameraComponent>();
    if (nullptr != cameraComp)
    {
        Camera* camera = cameraComp->GetCamera();
        camera->Rotate(cameraDelta);
    }
}

bool GameInputSystem::CanMove(const DAVA::Entity* entity) const
{
    const HealthComponent* healthComponent = entity->GetComponent<HealthComponent>();
    if (healthComponent && healthComponent->GetHealth() == 0)
    {
        return false;
    }

    const GameStunnableComponent* stunComp = entity->GetComponent<GameStunnableComponent>();
    if (stunComp && stunComp->IsStunned())
    {
        return false;
    }

    return true;
}
