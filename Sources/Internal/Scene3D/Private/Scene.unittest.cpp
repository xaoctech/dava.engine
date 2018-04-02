#include "UnitTests/UnitTests.h"
#include "Scene3D/Scene.h"
#include "Entity/SceneSystem.h"
#include "Entity/SingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

class MySystem : public SceneSystem
{
public:
    MySystem(Scene* scene)
        : SceneSystem(scene, ComponentMask())
    {
    }

    void PrepareForRemove() override
    {
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MySystem, SceneSystem)
    {
        ReflectionRegistrator<MySystem>::Begin()
        .End();
    }
};

class MyComponent : public SingleComponent
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MyComponent, SingleComponent)
    {
        ReflectionRegistrator<MyComponent>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

class UTSystem : public SceneSystem
{
public:
    UTSystem(Scene* scene)
        : SceneSystem(scene, ComponentMask())
    {
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(UTSystem, SceneSystem)
    {
        ReflectionRegistrator<UTSystem>::Begin()[M::Tags("UTSystem")]
        .Method("Process", &UTSystem::Process)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::NORMAL, -1337.42f)]
        .Method("ProcessFixed", &UTSystem::ProcessFixed)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, -42.1337f)]
        .ConstructorByPointer<Scene*>()
        .End();
    }

    void Process(float32) override
    {
        ++processCallsCount;
    }

    void ProcessFixed(float32) override
    {
        ++fixedProcessCallsCount;
    }

    uint32 processCallsCount = 0;
    uint32 fixedProcessCallsCount = 0;
};

DAVA_TESTCLASS (SceneTest)
{
    SceneTest()
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MyComponent);
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MySystem);
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UTSystem);
        GetEngineContext()->componentManager->RegisterComponent<MyComponent>();
        GetEngineContext()->systemManager->RegisterAllDerivedSceneSystemsRecursively();
    }

    DAVA_TEST (GetSystem)
    {
        Scene* scene = new Scene();
        SCOPE_EXIT
        {
            SafeRelease(scene);
        };

        MySystem* mySystem = new MySystem(scene);
        SCOPE_EXIT
        {
            delete mySystem;
        };

        TEST_VERIFY(scene->GetSystem<MySystem>() == nullptr);

        scene->AddSystem(mySystem);
        TEST_VERIFY(scene->GetSystem<MySystem>() == mySystem);

        scene->RemoveSystem(mySystem);
        TEST_VERIFY(scene->GetSystem<MySystem>() == nullptr);
    }

    DAVA_TEST (Process)
    {
        const uint32 updatesCount = 1e3;
        float32 fixedDelta = 1.f / 60.f;
        std::srand(time(nullptr));

        Scene* scene = nullptr;
        UTSystem* utSystem = nullptr;

        auto ResetScene = [&]()
        {
            SafeRelease(scene);
            scene = new Scene({ FastName("UTSystem") });
            scene->CreateSystemsByTags();

            scene->RemoveTag(FastName("base"));
            if (scene->HasTag(FastName("physics")))
            {
                scene->RemoveTag(FastName("physics"));
            }

            utSystem = scene->GetSystem<UTSystem>();
        };

        ResetScene();

        scene->SetPerformFixedProcessOnlyOnce(false);
        scene->SetFixedUpdateTime(fixedDelta);

        for (uint32 i = 0; i < updatesCount; ++i)
        {
            scene->Update(fixedDelta);
        }

        TEST_VERIFY(utSystem->processCallsCount == updatesCount);
        TEST_VERIFY(utSystem->fixedProcessCallsCount == updatesCount);

        ResetScene();

        scene->SetPerformFixedProcessOnlyOnce(false);
        scene->SetFixedUpdateTime(fixedDelta);

        float32 totalTime = 0.f;
        for (uint32 i = 0; i < updatesCount; ++i)
        {
            float32 updTime = static_cast<float32>(std::rand() % 1000) / 1000.f;
            totalTime += updTime;
            scene->Update(updTime);
        }

        TEST_VERIFY(utSystem->processCallsCount == updatesCount);

        uint32 expected = static_cast<uint32>(totalTime / fixedDelta);
        TEST_VERIFY(std::fabs(utSystem->fixedProcessCallsCount - expected) < 2);

        ResetScene();

        scene->SetPerformFixedProcessOnlyOnce(true);
        scene->SetFixedUpdateTime(fixedDelta);

        float32 updTimeLargerThanFixedDelta = fixedDelta * 3.f;
        for (uint32 i = 0; i < updatesCount; ++i)
        {
            scene->Update(updTimeLargerThanFixedDelta);
        }

        TEST_VERIFY(utSystem->processCallsCount == updatesCount);
        TEST_VERIFY(utSystem->fixedProcessCallsCount == updatesCount);

        ResetScene();

        scene->SetPerformFixedProcessOnlyOnce(true);
        scene->SetFixedUpdateTime(fixedDelta);

        totalTime = 0.f;
        float32 updTimeLessThanFixedDelta = fixedDelta / 3.f;
        for (uint32 i = 0; i < updatesCount; ++i)
        {
            scene->Update(updTimeLessThanFixedDelta);
        }

        TEST_VERIFY(utSystem->processCallsCount == updatesCount);

        expected = updatesCount / 3;
        TEST_VERIFY(std::fabs(utSystem->fixedProcessCallsCount - expected) < 2);
    }
};
