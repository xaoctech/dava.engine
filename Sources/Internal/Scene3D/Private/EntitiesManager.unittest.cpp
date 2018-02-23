#include <UnitTests/UnitTests.h>

#include <Entity/Component.h>
#include <Entity/SceneSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

using namespace DAVA;

static int32 entityAddCount = 0;
static int32 entityRemoveCount = 0;
static int32 entityDestroyCount = 0;

static int32 componentAddCount = 0;
static int32 componentRemoveCount = 0;
static int32 componentDestroyCount = 0;

class EMEntityComponent : public Component
{
public:
    ~EMEntityComponent()
    {
        entityDestroyCount++;
    }

    Component* Clone(Entity* toEntity) override
    {
        return new EMEntityComponent(*this);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EMEntityComponent, Component)
    {
        ReflectionRegistrator<EMEntityComponent>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

class EMComponentComponent : public Component
{
public:
    ~EMComponentComponent()
    {
        entityDestroyCount++;
    }

    Component* Clone(Entity* toEntity) override
    {
        return new EMComponentComponent(*this);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EMComponentComponent, Component)
    {
        ReflectionRegistrator<EMComponentComponent>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

class EMSystem : public SceneSystem
{
public:
    EMSystem(Scene* scene)
        : SceneSystem(scene, 0)
    {
        entityGroup = scene->AquireEntityGroup<EMEntityComponent>();
        entityGroup->onEntityAdded->Connect(this, &EMSystem::OnEntityAdded);
        entityGroup->onEntityRemoved->Connect(this, &EMSystem::OnEntityRemoved);

        componentGroup = scene->AquireComponentGroup<EMComponentComponent>();
        componentGroup->onComponentAdded->Connect(this, &EMSystem::OnComponentAdded);
        componentGroup->onComponentRemoved->Connect(this, &EMSystem::OnComponentRemoved);
    }

    ~EMSystem()
    {
        entityGroup->onEntityAdded->Disconnect(this);
        entityGroup->onEntityRemoved->Disconnect(this);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(EMSystem, SceneSystem)
    {
        ReflectionRegistrator<EMSystem>::Begin()[M::Tags("EntitiesManagerTest")]
        .ConstructorByPointer<Scene*>()
        .End();
    }

    void OnEntityAdded(Entity* e)
    {
        entityAddCount++;
    }

    void OnEntityRemoved(Entity* e)
    {
        entityRemoveCount++;
        TEST_VERIFY(e->GetRetainCount() == 1 || e->GetRetainCount() == 2); //entity manager must hold strong reference to 'e' during this call
    }

    void OnComponentAdded(EMComponentComponent* c)
    {
        componentAddCount++;
    }

    void OnComponentRemoved(EMComponentComponent* c)
    {
        componentRemoveCount++;
    }

    EntityGroup* entityGroup;
    ComponentGroup<EMComponentComponent>* componentGroup;
};

DAVA_TESTCLASS (EntitiesManagerTest)
{
    EntitiesManagerTest()
    {
        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EMSystem);
        GetEngineContext()->systemManager->RegisterAllDerivedSceneSystemsRecursively();

        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EMEntityComponent);
        GetEngineContext()->componentManager->RegisterComponent<EMEntityComponent>();

        DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EMComponentComponent);
        GetEngineContext()->componentManager->RegisterComponent<EMComponentComponent>();
    }

    DAVA_TEST (EntityGroupSignals)
    {
        entityAddCount = 0;
        entityRemoveCount = 0;
        entityDestroyCount = 0;

        Scene* scene = new Scene({ FastName("EntitiesManagerTest") });
        scene->CreateSystemsByTags();

        Entity* e = new Entity;
        scene->AddNode(e);

        e->AddComponent(new EMEntityComponent);

        TEST_VERIFY(entityAddCount == 1);
        TEST_VERIFY(entityRemoveCount == 0);
        scene->Update(0.1f);
        TEST_VERIFY(entityAddCount == 1);
        TEST_VERIFY(entityRemoveCount == 0);

        scene->RemoveNode(e);
        TEST_VERIFY(entityAddCount == 1);
        TEST_VERIFY(entityRemoveCount == 1);
        scene->Update(0.1f);
        TEST_VERIFY(entityAddCount == 1);
        TEST_VERIFY(entityRemoveCount == 1);

        scene->AddNode(e);
        TEST_VERIFY(entityAddCount == 2);
        TEST_VERIFY(entityRemoveCount == 1);
        scene->Update(0.1f);
        TEST_VERIFY(entityAddCount == 2);
        TEST_VERIFY(entityRemoveCount == 1);

        e->Release(); //scene still holds strong reference to 'e'

        scene->RemoveNode(e);
        TEST_VERIFY(entityAddCount == 2);
        TEST_VERIFY(entityRemoveCount == 2);
        TEST_VERIFY(entityDestroyCount == 1);
        scene->Update(0.1f);
        TEST_VERIFY(entityAddCount == 2);
        TEST_VERIFY(entityRemoveCount == 2);
        TEST_VERIFY(entityDestroyCount == 1);

        SafeRelease(scene);
    }

    DAVA_TEST (ComponentGroupSignals)
    {
        componentAddCount = 0;
        componentRemoveCount = 0;
        componentDestroyCount = 0;

        Scene* scene = new Scene({ FastName("EntitiesManagerTest") });
        scene->CreateSystemsByTags();

        Entity* e = new Entity;
        scene->AddNode(e);

        e->AddComponent(new EMComponentComponent);
        TEST_VERIFY(componentAddCount == 1);
        TEST_VERIFY(componentRemoveCount == 0);
        scene->Update(0.1f);
        TEST_VERIFY(componentAddCount == 1);
        TEST_VERIFY(componentRemoveCount == 0);

        SafeRelease(scene);
    }
};