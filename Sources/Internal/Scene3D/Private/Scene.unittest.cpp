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

DAVA_TESTCLASS (SceneTest)
{
    SceneTest()
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MyComponent);
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(MySystem);
        GetEngineContext()->componentManager->RegisterComponent<MyComponent>();
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

    DAVA_TEST (SingleComponent)
    {
        /*
        Scene* scene = new Scene();
        SCOPE_EXIT
        {
            SafeRelease(scene);
        };

        MyComponent* myComponent = new MyComponent();

        TEST_VERIFY(scene->GetSingleComponent<MyComponent>() == nullptr);

        scene->AddSingletonComponent(myComponent);
        TEST_VERIFY(scene->GetSingleComponent<MyComponent>() == myComponent);

        scene->RemoveSingletonComponent(myComponent);
        TEST_VERIFY(scene->GetSingleComponent<MyComponent>() == nullptr);
        */
    }
};
