#include "ExplosiveRocketComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

using namespace DAVA;

const uint32 ExplosiveRocketComponent::SPLIT_DISTANCE = 100;
const uint32 ExplosiveRocketComponent::MAX_DISTANCE = 250;
const float32 ExplosiveRocketComponent::MOVE_SPEED = 60.f;
const float32 ExplosiveRocketComponent::ROT_SPEED = 120.f * DEG_TO_RAD;

DAVA_VIRTUAL_REFLECTION_IMPL(ExplosiveRocketComponent)
{
    ReflectionRegistrator<ExplosiveRocketComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Distance", &ExplosiveRocketComponent::distance)[M::Replicable()]
    .Field("Stage", &ExplosiveRocketComponent::stage)[M::Replicable()]
    .Field("Shooter", &ExplosiveRocketComponent::shooterId)[M::Replicable(M::Privacy::PRIVATE)]
    .Field("Traget", &ExplosiveRocketComponent::targetId)[M::Replicable(M::Privacy::PRIVATE)]
    .End();
}

ExplosiveRocketComponent::ExplosiveRocketComponent()
{
}

ExplosiveRocketComponent::~ExplosiveRocketComponent()
{
}

Component* ExplosiveRocketComponent::Clone(Entity* toEntity)
{
    ExplosiveRocketComponent* component = new ExplosiveRocketComponent();
    component->SetEntity(toEntity);
    return component;
}

void ExplosiveRocketComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void ExplosiveRocketComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

ExplosiveRocketComponent::Stage ExplosiveRocketComponent::GetStage() const
{
    return stage;
}
void ExplosiveRocketComponent::SetStage(Stage stage_)
{
    stage = stage_;
}

uint32 ExplosiveRocketComponent::GetDistance() const
{
    return distance;
}

void ExplosiveRocketComponent::SetDistance(uint32 distance_)
{
    distance = distance_;
}