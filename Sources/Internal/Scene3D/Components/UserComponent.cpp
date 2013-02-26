#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{

UserComponent::UserComponent()
{ }

Component * UserComponent::Clone(SceneNode * toEntity)
{
	UserComponent *uc = new UserComponent();
	uc->SetEntity(toEntity);

	return uc;
}

void UserComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);
}

void UserComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Deserialize(archive, sceneFile);
}

}
