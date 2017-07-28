#include "WASDControllerSystem.h"

#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Render/Highlevel/Camera.h"

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Utils/Utils.h"

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/CharacterControllerComponent.h>
#endif

namespace DAVA
{
WASDControllerSystem::WASDControllerSystem(Scene* scene)
    : SceneSystem(scene)
    , moveSpeed(0.05f)
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

    KeyboardDevice& keyboard = InputSystem::Instance()->GetKeyboard();
    if (keyboard.IsKeyPressed(Key::LSHIFT) || keyboard.IsKeyPressed(Key::LCTRL) || keyboard.IsKeyPressed(Key::LALT) || keyboard.IsKeyPressed(Key::RALT) || keyboard.IsKeyPressed(Key::RCTRL))
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
            if (keyboard.IsKeyPressed(Key::KEY_W) || keyboard.IsKeyPressed(Key::UP))
            {
                moved = true;
                MoveForward(camera, entities[i], actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if (keyboard.IsKeyPressed(Key::KEY_S) || keyboard.IsKeyPressed(Key::DOWN))
            {
                moved = true;
                MoveForward(camera, entities[i], actualMoveSpeed, DIRECTION_INVERSE);
            }
            if (keyboard.IsKeyPressed(Key::KEY_D) || keyboard.IsKeyPressed(Key::RIGHT))
            {
                moved = true;
                MoveRight(camera, entities[i], actualMoveSpeed, DIRECTION_STRAIGHT);
            }
            if (keyboard.IsKeyPressed(Key::KEY_A) || keyboard.IsKeyPressed(Key::LEFT))
            {
                moved = true;
                MoveRight(camera, entities[i], actualMoveSpeed, DIRECTION_INVERSE);
            }

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
            CharacterControllerComponent* physicsController = static_cast<CharacterControllerComponent*>(entities[i]->GetComponent(Component::BOX_CHARACTER_CONTROLLER_COMPONENT));
            if (physicsController == nullptr)
            {
                CharacterControllerComponent* physicsController = static_cast<CharacterControllerComponent*>(entities[i]->GetComponent(Component::CAPSULE_CHARACTER_CONTROLLER_COMPONENT));
            }

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
    CharacterControllerComponent* physicsController = static_cast<CharacterControllerComponent*>(parentEntity->GetComponent(Component::BOX_CHARACTER_CONTROLLER_COMPONENT));
    if (physicsController == nullptr)
    {
        CharacterControllerComponent* physicsController = static_cast<CharacterControllerComponent*>(parentEntity->GetComponent(Component::CAPSULE_CHARACTER_CONTROLLER_COMPONENT));
    }

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
    CharacterControllerComponent* physicsController = static_cast<CharacterControllerComponent*>(parentEntity->GetComponent(Component::BOX_CHARACTER_CONTROLLER_COMPONENT));
    if (physicsController == nullptr)
    {
        CharacterControllerComponent* physicsController = static_cast<CharacterControllerComponent*>(parentEntity->GetComponent(Component::CAPSULE_CHARACTER_CONTROLLER_COMPONENT));
    }

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
