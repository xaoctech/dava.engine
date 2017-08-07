#include "Physics/VehicleWheelComponent.h"

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
float32 VehicleWheelComponent::GetRadius() const
{
    return radius;
}

void VehicleWheelComponent::SetRadius(float32 value)
{
    DVASSERT(value > 0.0f);
    radius = value;
}

float32 VehicleWheelComponent::GetWidth() const
{
    return width;
}

void VehicleWheelComponent::SetWidth(float32 value)
{
    DVASSERT(value > 0.0f);
    width = value;
}

void VehicleWheelComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetFloat("vehicleWheel.radius", radius);
    archive->SetFloat("vehicleWheel.width", width);
}

void VehicleWheelComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    radius = archive->GetFloat("vehicleWheel.radius", 0.5f);
    width = archive->GetFloat("vehicleWheel.width", 0.4f);
}

uint32 VehicleWheelComponent::GetType() const
{
    return Component::VEHICLE_WHEEL_COMPONENT;
}

Component* VehicleWheelComponent::Clone(Entity* toEntity)
{
    VehicleWheelComponent* result = new VehicleWheelComponent();
    result->SetRadius(radius);
    result->SetWidth(width);
    result->SetEntity(toEntity);

    return result;
}

DAVA_VIRTUAL_REFLECTION_IMPL(VehicleWheelComponent)
{
    ReflectionRegistrator<VehicleWheelComponent>::Begin()
    .ConstructorByPointer()
    .Field("Radius", &VehicleWheelComponent::GetRadius, &VehicleWheelComponent::SetRadius)[M::Range(0.01f, Any(), 0.5f)]
    .Field("Width", &VehicleWheelComponent::GetWidth, &VehicleWheelComponent::SetWidth)[M::Range(0.01f, Any(), 0.4f)]
    .End();
}
}