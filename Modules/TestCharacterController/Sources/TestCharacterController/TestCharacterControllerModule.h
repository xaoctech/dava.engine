#pragma once

#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
class Entity;
class TestCharacterMoveSystem;
class TestCharacterWeaponSystem;
class TestCharacterCameraSystem;
class TestCharacterControllerSystem;

class TestCharacterControllerModule : public IModule
{
public:
    TestCharacterControllerModule(Engine* engine);

    bool AddSceneSystems(Scene* scene);
    void RemoveSceneSystems(Scene* scene);
    void Shutdown() override;

    bool EnableController(Scene* scene, const Vector3& spawnPoint = Vector3(0.f, 0.f, 100.f));
    bool DisableController(Scene* scene);

    TestCharacterControllerSystem* GetCharacterControllerSystem(Scene* scene) const;

    static const FastName systemsTag;

protected:
    struct SceneContext
    {
        TestCharacterControllerSystem* characterControllerSystem = nullptr;
        TestCharacterMoveSystem* characterMoveSystem = nullptr;
        TestCharacterWeaponSystem* characterWeaponSystem = nullptr;
        TestCharacterCameraSystem* characterCameraSystem = nullptr;

        Entity* characterEntity = nullptr;
    };

    void CheckCharacterResources();

    Map<Scene*, SceneContext> activeControllers;
    Entity* testCharacterEntity = nullptr;

    DAVA_VIRTUAL_REFLECTION(TestCharacterControllerModule, IModule);
};
};
