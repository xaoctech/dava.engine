#include "SimpleVisibilitySystem.h"
#include "SimpleVisibilityShapeComponent.h"

#include "Debug/ProfilerCPU.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Scene.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "Logger/Logger.h"

#include <algorithm>

using namespace DAVA;

namespace SimpleVisibilityDetail
{
constexpr uint8 DEFAULT_UPDATE_PERIOD = 8u;
};

DAVA_VIRTUAL_REFLECTION_IMPL(SimpleVisibilitySystem)
{
    ReflectionRegistrator<SimpleVisibilitySystem>::Begin()[M::Tags("server", "simple_visibility")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &SimpleVisibilitySystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 21.2f)]
    .End();
}

SimpleVisibilitySystem::SimpleVisibilitySystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<NetworkReplicationComponent>())
    , updatePeriod(SimpleVisibilityDetail::DEFAULT_UPDATE_PERIOD)
{
    observerGroup = GetScene()->AquireComponentGroup<ObserverComponent, ObserverComponent>();
    for (ObserverComponent* c : observerGroup->components)
    {
        AddObserver(c);
    }
    observerGroup->onComponentAdded->Connect(this, &SimpleVisibilitySystem::AddObserver);
    observerGroup->onComponentRemoved->Connect(this, &SimpleVisibilitySystem::RemoveObserver);

    observableGroup = GetScene()->AquireComponentGroup<ObservableComponent, ObservableComponent,
                                                       ObservableIdComponent, SimpleVisibilityShapeComponent>();
    for (ObservableComponent* c : observableGroup->components)
    {
        AddObservable(c);
    }
    observableGroup->onComponentAdded->Connect(this, &SimpleVisibilitySystem::AddObservable);
    observableGroup->onComponentRemoved->Connect(this, &SimpleVisibilitySystem::RemoveObservable);
}

SimpleVisibilitySystem::~SimpleVisibilitySystem()
{
    observerGroup->onComponentAdded->Disconnect(this);
    observerGroup->onComponentRemoved->Disconnect(this);
    observableGroup->onComponentAdded->Disconnect(this);
    observableGroup->onComponentRemoved->Disconnect(this);
}

void SimpleVisibilitySystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("SimpleVisibilitySystem::ProcessFixed");

    auto updateObservable = [this](ObservableItem& observable)
    {
        const Vector3& observablePos = observable.transform->GetPosition();
        for (ObserverItem& observer : observers)
        {
            const Vector3& observerPos = observer.transform->GetPosition();
            float32 distSqr = DistanceSquared(observerPos, observablePos);
            float maxVisibilityRadius = observer.comp->maxVisibilityRadius;
            bool isVisible = (distSqr <= maxVisibilityRadius * maxVisibilityRadius);
            observer.comp->SetVisible(observable.id, isVisible);
        }
    };

    for (ObservableItem& observable : observables)
    {
        if ((observable.id + frameOffset) % updatePeriod == 0)
        {
            updateObservable(observable);
        }
    }

    for (ObservableItem& observable : justAddedObservables)
    {
        updateObservable(observable);
    }

    justAddedObservables.clear();
    frameOffset++;
}

void SimpleVisibilitySystem::AddObserver(ObserverComponent* comp)
{
    comp->Reset();

    observers.emplace_back();
    ObserverItem& observer = observers.back();
    observer.comp = comp;
    observer.transform = comp->GetEntity()->GetComponent<TransformComponent>();
}

void SimpleVisibilitySystem::RemoveObserver(ObserverComponent* comp)
{
    auto observerIt = std::find_if(observers.begin(), observers.end(),
                                   [comp](const ObserverItem& item) { return comp == item.comp; });

    *observerIt = std::move(observers.back());
    observers.pop_back();
}

void SimpleVisibilitySystem::AddObservable(ObservableComponent* comp)
{
    Entity* entity = comp->GetEntity();

    observables.emplace_back();
    ObservableItem& observable = observables.back();
    observable.transform = entity->GetComponent<TransformComponent>();
    observable.id = entity->GetComponent<ObservableIdComponent>()->id;

    justAddedObservables.push_back(observable);
}

void SimpleVisibilitySystem::RemoveObservable(ObservableComponent* comp)
{
    ObservableId id = comp->GetEntity()->GetComponent<ObservableIdComponent>()->id;
    auto observableIt = std::find_if(observables.begin(), observables.end(),
                                     [comp, id](const ObservableItem& item) { return id == item.id; });

    for (ObserverItem& observer : observers)
    {
        observer.comp->SetVisible(id, false);
    }

    *observableIt = std::move(observables.back());
    observables.pop_back();
}

void SimpleVisibilitySystem::SetUpdatePeriod(uint8 period)
{
    updatePeriod = period;
}

uint8 SimpleVisibilitySystem::GetUpdatePeriod() const
{
    return updatePeriod;
}
