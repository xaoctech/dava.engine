#include "ObservableIdSystem.h"

#include "Scene3D/Scene.h"
#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ObservableIdSystem)
{
    ReflectionRegistrator<ObservableIdSystem>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ObservableIdSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 21.1f)]
    .End();
}

ObservableIdSystem::ObservableIdSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
    , nextNewObservableId(0)
{
    observableEntityGroup = GetScene()->AquireEntityGroup<ObservableComponent>();
    for (Entity* e : observableEntityGroup->GetEntities())
    {
        AddObservable(e);
    }
    observableEntityGroup->onEntityAdded->Connect(this, &ObservableIdSystem::AddObservable);
    observableEntityGroup->onEntityRemoved->Connect(this, &ObservableIdSystem::RemoveObservable);
}

ObservableIdSystem::~ObservableIdSystem()
{
    observableEntityGroup->onEntityAdded->Disconnect(this);
    observableEntityGroup->onEntityRemoved->Disconnect(this);
}

void ObservableIdSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (Entity* entity : addedEntities)
    {
        ObservableIdComponent* idComp = new ObservableIdComponent();
        idComp->id = GenerateObservableId();
        entity->AddComponent(idComp);
    }
    addedEntities.clear();
}

ObservableId ObservableIdSystem::GenerateObservableId()
{
    ObservableId newId;
    if (freeObservableIds.empty())
    {
        newId = nextNewObservableId++;
        DVASSERT(newId < MAX_OBSERVABLES_COUNT);
    }
    else
    {
        newId = freeObservableIds.back();
        freeObservableIds.pop_back();
    }
    return newId;
}

void ObservableIdSystem::FreeObservableId(ObservableId observableId)
{
    freeObservableIds.push_back(observableId);
}

void ObservableIdSystem::AddObservable(Entity* entity)
{
    ObservableIdComponent* idComp = entity->GetComponent<ObservableIdComponent>();
    if (idComp)
    {
        idComp->id = GenerateObservableId();
    }
    else
    {
        addedEntities.push_back(entity);
    }
}

void ObservableIdSystem::RemoveObservable(Entity* entity)
{
    ObservableIdComponent* idComp = entity->GetComponent<ObservableIdComponent>();
    FreeObservableId(idComp->id);
}
