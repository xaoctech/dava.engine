#include "Components/ShooterMirroredCharacterComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

REGISTER_CLASS(ShooterMirroredCharacterComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterMirroredCharacterComponent)
{
    using namespace DAVA;

    ReflectionRegistrator<ShooterMirroredCharacterComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("MirrorIsMaster", &ShooterMirroredCharacterComponent::GetMirrorIsMaster, &ShooterMirroredCharacterComponent::SetMirrorIsMaster)[M::Replicable(), M::HiddenField()]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* ShooterMirroredCharacterComponent::Clone(DAVA::Entity* toEntity)
{
    ShooterMirroredCharacterComponent* component = new ShooterMirroredCharacterComponent();
    component->SetMirrorIsMaster(mirrorIsMaster);
    component->SetEntity(toEntity);
    return component;
}

void ShooterMirroredCharacterComponent::SetMirrorIsMaster(bool value)
{
    mirrorIsMaster = value;
}

bool ShooterMirroredCharacterComponent::GetMirrorIsMaster() const
{
    return mirrorIsMaster;
}
