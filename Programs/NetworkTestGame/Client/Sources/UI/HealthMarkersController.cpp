#include "UI/HealthMarkersController.h"
#include "Components/HealthComponent.h"

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <UI/Scene3D/UIEntityMarkerComponent.h>
#include <UI/Scene3D/UIEntityMarkersContainerComponent.h>
#include <UI/UIControl.h>
#include <UI/Text/UITextComponent.h>

#include <NetworkCore/Scene3D/Components/NetworkReplicationComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>

HealthMarkersController::HealthMarkersController(DAVA::UIControl* container_, DAVA::UIControl* prototype_, DAVA::Scene* scene_)
{
    DVASSERT(container_);
    DVASSERT(prototype_);
    DVASSERT(scene_);

    container = container_;
    prototype = prototype_;

    container->GetOrCreateComponent<DAVA::UIEntityMarkersContainerComponent>()->SetCustomStrategy(&HealthMarkersController::CustomStrategy);

    group = scene_->AquireComponentGroup<HealthComponent>();
    group->onComponentAdded->Connect(this, &HealthMarkersController::OnAdd);
    group->onComponentRemoved->Connect(this, &HealthMarkersController::OnRemove);
}

HealthMarkersController::~HealthMarkersController()
{
    group->onComponentAdded->Disconnect(this);
    group->onComponentRemoved->Disconnect(this);
}

void HealthMarkersController::Process()
{
    using namespace DAVA;

    auto it = componentsOnAdd.begin();
    auto endIt = componentsOnAdd.end();
    while (it != endIt)
    {
        HealthComponent* c = *it;
        Entity* entity = c->GetEntity();
        Entity* markerEntity = entity->FindByName("Marker");
        if (markerEntity)
        {
            NetworkPlayerID id = entity->GetComponent<NetworkReplicationComponent>()->GetNetworkPlayerID();
            RefPtr<UIControl> marker = prototype->SafeClone();
            marker->GetOrCreateComponent<UIEntityMarkerComponent>()->SetTargetEntity(markerEntity);
            marker->FindByPath("PlayerId")->GetComponent<UITextComponent>()->SetText(std::to_string(id));
            container->AddControl(marker.Get());

            it = componentsOnAdd.erase(it);
            endIt = componentsOnAdd.end();
            continue;
        }
        ++it;
    }
}

void HealthMarkersController::OnAdd(HealthComponent* c)
{
    componentsOnAdd.push_back(c);
}

void HealthMarkersController::OnRemove(HealthComponent* c)
{
    componentsOnAdd.erase(std::remove(componentsOnAdd.begin(), componentsOnAdd.end(), c), componentsOnAdd.end());

    using namespace DAVA;
    if (container)
    {
        for (const auto& marker : container->GetChildren())
        {
            UIEntityMarkerComponent* mComponent = marker->GetOrCreateComponent<UIEntityMarkerComponent>();
            if (mComponent->GetTargetEntity() == c->GetEntity())
            {
                marker->RemoveFromParent();
                break;
            }
        }
    }
}

void HealthMarkersController::CustomStrategy(DAVA::UIControl* ctrl, DAVA::UIEntityMarkersContainerComponent*, DAVA::UIEntityMarkerComponent* emc)
{
    using namespace DAVA;
    Entity* markerEntity = emc->GetTargetEntity();
    Entity* healthEntityOpt = markerEntity->GetParent();
    while (healthEntityOpt)
    {
        HealthComponent* healthComponent = healthEntityOpt->GetComponent<HealthComponent>();
        if (healthComponent)
        {
            ctrl->SetTag(static_cast<int32>(healthComponent->GetPercentage() * 100));
            break;
        }
        healthEntityOpt = healthEntityOpt->GetParent();
    }
}
