#include "WASDControllerSystem.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "Input/Keyboard.h"

#include "Utils/Utils.h"

#include "DeviceManager/DeviceManager.h"

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/PhysicsHelpers.h>
#endif

namespace DAVA
{
WASDControllerSystem::WASDControllerSystem(Scene* scene)
    : SceneSystem(scene)
    , moveSpeed(1.f)
{
}

WASDControllerSystem::~WASDControllerSystem()
{
}

void WASDControllerSystem::AddEntity(Entity* entity)
{
    DVASSERT(GetCamera(entity) != NULL && "Right now system works with camera only");

    entities.push_back(entity);
}

void WASDControllerSystem::RemoveEntity(Entity* entity)
{
    const bool removeResult = FindAndRemoveExchangingWithLast(entities, entity);
    DVASSERT(removeResult);
}

void WASDControllerSystem::Process(float32 timeElapsed)
{
    float32 actualMoveSpeed = moveSpeed * timeElapsed;

    const uint32 size = static_cast<uint32>(entities.size());
    if (0 == size)
        return;

    Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    if (keyboard == nullptr)
    {
        return;
    }

    if (keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() ||
        keyboard->GetKeyState(eInputElements::KB_RCTRL).IsPressed() ||
        keyboard->GetKeyState(eInputElements::KB_LCTRL).IsPressed() ||
        keyboard->GetKeyState(eInputElements::KB_RALT).IsPressed() ||
        keyboard->GetKeyState(eInputElements::KB_LALT).IsPressed())
    {
        return;
    }

    for (uint32 i = 0; i < size; ++i)
    {
        Camera* camera = GetCamera(entities[i]);
        if ((camera != nullptr) && (camera == GetScene()->GetDrawCamera()))
        {
            // TODO: Character controller input is temporarily handled here for convinience
            // Should be discussed

            bool moved = false;
            if (keyboard->GetKeyState(eInputElements::KB_W).IsPressed() || keyboard->GetKeyState(eInputElements::KB_UP).IsPressed())
            {
                moved = true;
                MoveForward(camera, entities[i], actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if (keyboard->GetKeyState(eInputElements::KB_S).IsPressed() || keyboard->GetKeyState(eInputElements::KB_DOWN).IsPressed())
            {
                moved = true;
                MoveForward(camera, entities[i], actualMoveSpeed, DIRECTION_INVERSE);
            }
            if (keyboard->GetKeyState(eInputElements::KB_D).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RIGHT).IsPressed())
            {
                moved = true;
                MoveRight(camera, entities[i], actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if (keyboard->GetKeyState(eInputElements::KB_A).IsPressed() || keyboard->GetKeyState(eInputElements::KB_LEFT).IsPressed())
            {
                moved = true;
                MoveRight(camera, entities[i], actualMoveSpeed, DIRECTION_INVERSE);
            }

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
            CharacterControllerComponent* physicsController = GetCharacterControllerComponent(entities[i]);

            if (physicsController != nullptr)
            {
                if (!moved)
                {
                    physicsController->SimpleMove(Vector3::Zero);
                }

                const Vector3& dir = camera->GetDirection();

                camera->SetPosition(entities[i]->GetWorldTransform().GetTranslationVector());
                camera->SetDirection(dir);
            }
#endif
        }
    }
}

void WASDControllerSystem::MoveForward(Camera* camera, Entity* parentEntity, float32 speed, eDirection direction)
{
    Vector3 pos = camera->GetPosition();
    const Vector3& dir = camera->GetDirection();

    const Vector3 displacement = dir * speed * static_cast<float32>(direction);
    
#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    CharacterControllerComponent* physicsController = GetCharacterControllerComponent(parentEntity);

    if (physicsController != nullptr)
    {
        physicsController->SimpleMove(displacement);
    }
    else
    {
        pos += displacement;
        camera->SetPosition(pos);
        camera->SetDirection(dir);
    }
#else
    pos += displacement;
    camera->SetPosition(pos);
    camera->SetDirection(dir);
#endif
}

void WASDControllerSystem::MoveRight(Camera* camera, Entity* parentEntity, float32 speed, eDirection direction)
{
    Vector3 pos = camera->GetPosition();
    Vector3 right = -camera->GetLeft();
    const Vector3& dir = camera->GetDirection();

    const Vector3 displacement = right * speed * static_cast<float32>(direction);

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    CharacterControllerComponent* physicsController = GetCharacterControllerComponent(parentEntity);

    if (physicsController != nullptr)
    {
        physicsController->SimpleMove(displacement);
    }
    else
    {
        pos += displacement;
        camera->SetPosition(pos);
        camera->SetDirection(dir);
    }
#else
    pos += displacement;
    camera->SetPosition(pos);
    camera->SetDirection(dir);
#endif
}
};
