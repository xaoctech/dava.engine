#include "ShooterRocketComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"

using namespace DAVA;

const uint32 ShooterRocketComponent::SPLIT_DISTANCE = 20;
const uint32 ShooterRocketComponent::MAX_DISTANCE = 400;
const float32 ShooterRocketComponent::MOVE_SPEED = 8.f;
const float32 ShooterRocketComponent::ROT_SPEED = 60.f * DEG_TO_RAD;

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterRocketComponent)
{
    ReflectionRegistrator<ShooterRocketComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Distance", &ShooterRocketComponent::distance)[M::Replicable()]
    .Field("Stage", &ShooterRocketComponent::stage)[M::Replicable()]
    .Field("ShooterID", &ShooterRocketComponent::shooterId)[M::Replicable()]
    .Field("TragetID", &ShooterRocketComponent::targetId)[M::Replicable()]
    .End();
}

ShooterRocketComponent::ShooterRocketComponent()
{
}

ShooterRocketComponent::~ShooterRocketComponent()
{
}

Component* ShooterRocketComponent::Clone(Entity* toEntity)
{
    ShooterRocketComponent* component = new ShooterRocketComponent();
    component->SetEntity(toEntity);
    return component;
}

void ShooterRocketComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void ShooterRocketComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

ShooterRocketComponent::Stage ShooterRocketComponent::GetStage() const
{
    return stage;
}
void ShooterRocketComponent::SetStage(Stage stage_)
{
    stage = stage_;
}

uint32 ShooterRocketComponent::GetDistance() const
{
    return distance;
}

void ShooterRocketComponent::SetDistance(uint32 distance_)
{
    distance = distance_;
}
