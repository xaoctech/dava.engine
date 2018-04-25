#include "TestCharacterController/TestCharacterControllerModule.h"
#include "TestCharacterController/TestCharacterControllerSystems.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Math/Transform.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/MotionSystem.h>
#include <Scene3D/Systems/RenderUpdateSystem.h>
#include <Scene3D/Systems/TransformSystem.h>

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>
#include <Physics/PhysicsSystem.h>
#endif

namespace DAVA
{
const FastName TestCharacterControllerModule::systemsTag = FastName("test_character_controller");

DAVA_VIRTUAL_REFLECTION_IMPL(TestCharacterControllerModule)
{
    ReflectionRegistrator<TestCharacterControllerModule>::Begin()
    .End();
}

TestCharacterControllerModule::TestCharacterControllerModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TestCharacterControllerModule);
}

void TestCharacterControllerModule::CheckCharacterResources()
{
    if (testCharacterEntity == nullptr)
    {
        ScopedPtr<Scene> characterScene(new Scene());
        characterScene->LoadScene("~res:/TestCharacterControllerModule/character/character_mesh.sc2");

        Entity* characterSourceEntity = characterScene->FindByName("character");
        if (characterSourceEntity != nullptr)
        {
            testCharacterEntity = new Entity();

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
            testCharacterEntity->AddComponent(new CapsuleCharacterControllerComponent());
#endif

            ScopedPtr<Entity> characterMeshEntity(characterSourceEntity->Clone());
            characterMeshEntity->SetName("Character");
            characterMeshEntity->AddComponent(new MotionComponent());
            GetMotionComponent(characterMeshEntity)->SetDescriptorPath("~res:/TestCharacterControllerModule/character/character_motion.yaml");
            testCharacterEntity->AddNode(characterMeshEntity);

            ScopedPtr<Scene> weaponScene(new Scene());
            weaponScene->LoadScene("~res:/TestCharacterControllerModule/character/weapon_mesh.sc2");
            Entity* m4Entity = weaponScene->FindByName("weapon");
            if (m4Entity != nullptr)
            {
                ScopedPtr<Entity> weaponEntity(m4Entity->Clone());
                weaponEntity->SetName("Weapon");
                characterMeshEntity->AddNode(weaponEntity);
            }
        }
    }
}

bool TestCharacterControllerModule::AddSceneSystems(Scene* scene)
{
    CheckCharacterResources();

    if (testCharacterEntity == nullptr)
    {
        return false;
    }

    if (activeControllers.count(scene) != 0)
    {
        return false;
    }

    SceneContext& context = activeControllers[scene];

    if (!scene->GetTags().empty())
    {
        DVASSERT(!scene->HasTags(systemsTag));
        scene->AddTags(systemsTag);
    }
    else
    {
        scene->AddSystemManually(Type::Instance<TestCharacterControllerSystem>());
        scene->AddSystemManually(Type::Instance<TestCharacterWeaponSystem>());
        scene->AddSystemManually(Type::Instance<TestCharacterCameraSystem>());
#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
        scene->AddSystemManually(Type::Instance<TestCharacterMoveSystem>());
#endif
    }

    context.characterControllerSystem = scene->GetSystem<TestCharacterControllerSystem>();
#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    context.characterMoveSystem = scene->GetSystem<TestCharacterMoveSystem>();
#endif
    context.characterWeaponSystem = scene->GetSystem<TestCharacterWeaponSystem>();
    context.characterCameraSystem = scene->GetSystem<TestCharacterCameraSystem>();

    return true;
}

void TestCharacterControllerModule::RemoveSceneSystems(Scene* scene)
{
    if (scene->HasTags(systemsTag))
    {
        scene->RemoveTags(systemsTag);
    }
    else
    {
        scene->RemoveSystemManually(Type::Instance<TestCharacterControllerSystem>());
        scene->RemoveSystemManually(Type::Instance<TestCharacterWeaponSystem>());
        scene->RemoveSystemManually(Type::Instance<TestCharacterCameraSystem>());
#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
        scene->RemoveSystemManually(Type::Instance<TestCharacterMoveSystem>());
#endif
    }

    SceneContext& context = activeControllers[scene];

    context.characterControllerSystem = nullptr;
    context.characterMoveSystem = nullptr;
    context.characterWeaponSystem = nullptr;
    context.characterCameraSystem = nullptr;
}

void TestCharacterControllerModule::Shutdown()
{
    SafeRelease(testCharacterEntity);
}

bool TestCharacterControllerModule::EnableController(Scene* scene, const Vector3& spawnPoint)
{
    AddSceneSystems(scene);

    if (testCharacterEntity == nullptr)
    {
        return false;
    }

    SceneContext& context = activeControllers[scene];

    context.characterEntity = testCharacterEntity->Clone();
    TransformComponent* tc = context.characterEntity->GetComponent<TransformComponent>();
    tc->SetLocalTranslation(spawnPoint);
    scene->AddNode(context.characterEntity);

    context.characterControllerSystem->SetCharacterEntity(context.characterEntity);

    return true;
}

bool TestCharacterControllerModule::DisableController(Scene* scene)
{
    if (activeControllers.count(scene) == 0)
    {
        return false;
    }

    SceneContext& context = activeControllers[scene];

    context.characterControllerSystem->SetCharacterEntity(nullptr);

    RemoveSceneSystems(scene);

    if (context.characterEntity != nullptr)
    {
        scene->RemoveNode(context.characterEntity);
        SafeRelease(context.characterEntity);
    }

    activeControllers.erase(scene);

    return true;
}

TestCharacterControllerSystem* TestCharacterControllerModule::GetCharacterControllerSystem(Scene* scene) const
{
    auto found = activeControllers.find(scene);

    if (found != activeControllers.end())
    {
        return found->second.characterControllerSystem;
    }

    return nullptr;
}

} // namespace DAVA
