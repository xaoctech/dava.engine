#include "Scene3D/Components/BulletComponent.h"
#include "Base/BaseObject.h"

namespace DAVA
{

BulletComponent::BulletComponent()
:	bulletObject(0)
{

}

BulletComponent::~BulletComponent()
{
}

Component * BulletComponent::Clone(SceneNode * toEntity)
{
	BulletComponent * newComponent = new BulletComponent();
	newComponent->SetEntity(toEntity);
	//bulletObject is intentionally not cloned
	return newComponent;
}

void BulletComponent::SetBulletObject(BaseObject * _bulletObject)
{
	bulletObject = _bulletObject;
}

BaseObject * BulletComponent::GetBulletObject()
{
	return bulletObject;
}



}
