#include "Scene3D/Components/UpdatableComponent.h"
//#include "Scene3D/"

namespace DAVA
{

UpdatableComponent::UpdatableComponent()
:	updatableObject(0)
{
}

Component * UpdatableComponent::Clone()
{
	UpdatableComponent * newComponent = new UpdatableComponent();
	newComponent->SetUpdatableObject(updatableObject);
	
	return newComponent;
}

void UpdatableComponent::SetUpdatableObject(IUpdatable * _updatableObject)
{
	updatableObject = _updatableObject;

	//entity->GetScene()->updatableSystem->RemoveEntity(entity);
}

IUpdatable * UpdatableComponent::GetUpdatableObject()
{
	return updatableObject;
}

}