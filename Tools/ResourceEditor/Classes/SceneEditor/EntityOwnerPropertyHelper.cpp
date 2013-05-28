#include "EntityOwnerPropertyHelper.h"
#include "EditorSettings.h"

namespace DAVA {

void EntityOwnerPropertyHelper::UpdateEntityOwner(Entity* entity)
{
	entity->SetDesignerName(EditorSettings::Instance()->GetDesignerName());
	entity->UpdateModificationTime();
}

};