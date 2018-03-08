#include "NetworkFrequencySystem.h"
#include "ObservableIdComponent.h"

#include "Scene3D/Scene.h"
#include "Reflection/ReflectionRegistrator.h"

#include <algorithm>

using namespace DAVA;

namespace NetworkFrequencyDetail
{
constexpr uint8 DEFAULT_UPDATE_PERIOD = 8u;
constexpr uint8 MAX_SEND_PERIOD = 8u;
};

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkFrequencySystem)
{
    ReflectionRegistrator<NetworkFrequencySystem>::Begin()[M::Tags("server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &NetworkFrequencySystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 21.4f)]
    .End();
}

NetworkFrequencySystem::NetworkFrequencySystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
    , updatePeriod(NetworkFrequencyDetail::DEFAULT_UPDATE_PERIOD)
{
    observerGroup = GetScene()->AquireComponentGroup<ObserverComponent, ObserverComponent, NetworkPlayerComponent>();
    for (ObserverComponent* c : observerGroup->components)
    {
        AddObserver(c);
    }
    observerGroup->onComponentAdded->Connect(this, &NetworkFrequencySystem::AddObserver);
    observerGroup->onComponentRemoved->Connect(this, &NetworkFrequencySystem::AddObserver);

    observableGroup = GetScene()->AquireComponentGroup<ObservableComponent, ObservableComponent, ObservableIdComponent>();
    for (ObservableComponent* c : observableGroup->components)
    {
        AddObservable(c);
    }
    observableGroup->onComponentAdded->Connect(this, &NetworkFrequencySystem::AddObservable);
    observableGroup->onComponentRemoved->Connect(this, &NetworkFrequencySystem::RemoveObservable);
}

NetworkFrequencySystem::~NetworkFrequencySystem()
{
    observerGroup->onComponentAdded->Disconnect(this);
    observerGroup->onComponentRemoved->Disconnect(this);
    observableGroup->onComponentAdded->Disconnect(this);
    observableGroup->onComponentRemoved->Disconnect(this);
}

void NetworkFrequencySystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    for (auto item : observableTransforms)
    {
        ObservableId observableId = item.first;
        if ((observableId + frameOffset) % updatePeriod == 0)
        {
            TransformComponent* observableTransform = item.second;

            toObserveOnCurrFrame.emplace_back();
            ObservableCache& cache = toObserveOnCurrFrame.back();

            cache.entity = observableTransform->GetEntity();
            cache.position = observableTransform->GetPosition();
            cache.id = observableId;
        }
    }

    for (ObserverItem& observer : observers)
    {
        ObserverComponent* observerComp = observer.comp;

        ObserverCache observerCache;
        observerCache.position = observer.transform->GetPosition();
        observerCache.playerComp = observer.playerComp;
        observerCache.networkPeriodByObservableId = &observer.networkPeriodByObservableId;

        for (ObservableCache& observableCache : toObserveOnCurrFrame)
        {
            UpdateNetworkPeriod(observerCache, observableCache, observerComp->IsVisible(observableCache.id));
        }

        for (ObservableId updatedId : observerComp->GetUpdatedObservablesIds())
        {
            auto it = observableTransforms.find(updatedId);
            if (it == observableTransforms.end())
            {
                continue;
            }

            TransformComponent* observableTransform = it->second;
            ObservableCache observableCache;
            observableCache.entity = observableTransform->GetEntity();
            observableCache.position = observableTransform->GetPosition();
            observableCache.id = updatedId;

            UpdateNetworkPeriod(observerCache, observableCache, observerComp->IsVisible(updatedId));
        }

        observerComp->ClearCache();
    }

    toObserveOnCurrFrame.clear();
    frameOffset++;
}

void NetworkFrequencySystem::UpdateNetworkPeriod(ObserverCache& observerCache, ObservableCache& observableCache, bool isVisible)
{
    if (!isVisible)
    {
        Hide(observerCache, observableCache);
        return;
    }

    float32 sqrDist = DistanceSquared(observerCache.position, observableCache.position);

    uint8 newPeriod;
    if (sqrDist < nwPeriodIncreaseDistanceSqr)
    {
        newPeriod = 1;
    }
    else if (sqrDist > maxAOISqr)
    {
        newPeriod = 0;
    }
    else
    {
        uint32 rawPeriod = static_cast<uint32>(std::ceil(std::sqrt(sqrDist / nwPeriodIncreaseDistanceSqr)));
        newPeriod = std::min(static_cast<uint32>(NetworkFrequencyDetail::MAX_SEND_PERIOD), rawPeriod);
    }

    Vector<uint8>& currPeriods = *observerCache.networkPeriodByObservableId;
    if (newPeriod != currPeriods[observableCache.id])
    {
        currPeriods[observableCache.id] = newPeriod;
        observerCache.playerComp->SetSendPeriod(observableCache.entity, newPeriod);
    }
}

void NetworkFrequencySystem::Hide(ObserverCache& observerCache, ObservableCache& observableCache)
{
    Vector<uint8>& periods = *observerCache.networkPeriodByObservableId;
    if (periods[observableCache.id] != 0)
    {
        periods[observableCache.id] = 0;
        observerCache.playerComp->SetSendPeriod(observableCache.entity, 0);
    }
}

void NetworkFrequencySystem::AddObserver(ObserverComponent* comp)
{
    Entity* entity = comp->GetEntity();

    observers.emplace_back();
    ObserverItem& observer = observers.back();
    observer.comp = comp;
    observer.transform = entity->GetComponent<TransformComponent>();
    observer.playerComp = entity->GetComponent<NetworkPlayerComponent>();
    observer.networkPeriodByObservableId.resize(MAX_OBSERVABLES_COUNT, 0);
}

void NetworkFrequencySystem::RemoveObserver(ObserverComponent* comp)
{
    auto observerIt = std::find_if(observers.begin(), observers.end(),
                                   [comp](const ObserverItem& item) { return comp == item.comp; });

    NetworkPlayerComponent* playerComp = observerIt->playerComp;
    Vector<uint8>& periods = observerIt->networkPeriodByObservableId;

    for (auto item : observableTransforms)
    {
        if (periods[item.first] != 0)
        {
            playerComp->SetSendPeriod(item.second->GetEntity(), 0);
        }
    }

    *observerIt = std::move(observers.back());
    observers.pop_back();
}

void NetworkFrequencySystem::AddObservable(ObservableComponent* comp)
{
    Entity* entity = comp->GetEntity();
    ObservableId observableId = entity->GetComponent<ObservableIdComponent>()->id;

    observableTransforms[observableId] = entity->GetComponent<TransformComponent>();
}

void NetworkFrequencySystem::RemoveObservable(ObservableComponent* comp)
{
    Entity* entity = comp->GetEntity();
    ObservableId observableId = entity->GetComponent<ObservableIdComponent>()->id;

    for (ObserverItem& observer : observers)
    {
        if (observer.networkPeriodByObservableId[observableId] != 0)
        {
            observer.networkPeriodByObservableId[observableId] = 0;
            observer.playerComp->SetSendPeriod(entity, 0);
        }
    }

    observableTransforms.erase(observableId);
}

void NetworkFrequencySystem::SetMaxAOI(float32 maxAOI_)
{
    maxAOISqr = maxAOI_ * maxAOI_;
}

float32 NetworkFrequencySystem::GetMaxAOI() const
{
    return sqrtf(maxAOISqr);
}

void NetworkFrequencySystem::SetNetworkPeriodIncreaseDistance(float32 periodIncreaseDistance_)
{
    DVASSERT(periodIncreaseDistance_ > 0.f);
    nwPeriodIncreaseDistanceSqr = periodIncreaseDistance_ * periodIncreaseDistance_;
}

float32 NetworkFrequencySystem::GetNetworkPeriodIncreaseDistance() const
{
    return sqrtf(nwPeriodIncreaseDistanceSqr);
}

void NetworkFrequencySystem::SetUpdatePeriod(uint8 period)
{
    updatePeriod = period;
}

uint8 NetworkFrequencySystem::GetUpdatePeriod() const
{
    return updatePeriod;
}
