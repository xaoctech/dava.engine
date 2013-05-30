#include "SetSwitchIndexHelper.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "Scene3D/Entity.h"

namespace DAVA
{

void SetSwitchIndexHelper::ProcessSwitchIndexUpdate(uint32 value, eSET_SWITCH_INDEX state, Set<Entity*>& entities, Map<SwitchComponent *, int32>& originalIndexes)
{
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		
		List<Entity*> switchComponents;
		if( SetSwitchIndexHelper::FOR_SELECTED == state)
		{
			Entity *selectedNode = SceneDataManager::Instance()->SceneGetSelectedNode(sceneData);
			if(NULL != selectedNode)
			{
				selectedNode->FindComponentsByTypeRecursive(Component::SWITCH_COMPONENT, switchComponents);
			}
		}
		if( SetSwitchIndexHelper::FOR_SCENE == state)
		{
			sceneData->GetScene()->FindComponentsByTypeRecursive(Component::SWITCH_COMPONENT, switchComponents);
		}
		
		for(List<Entity*>::const_iterator it = switchComponents.begin(); it != switchComponents.end(); ++it)
		{
			Entity* en = *it;
			SwitchComponent * switchComponent = cast_if_equal<SwitchComponent*>(en->GetComponent(Component::SWITCH_COMPONENT));
			if(NULL == switchComponent)
			{
				continue;
			}

			int32 originalIndex = switchComponent->GetSwitchIndex();
			if(value == originalIndex)
			{
				continue;
			}
			
			originalIndexes[switchComponent] = switchComponent->GetSwitchIndex();
			switchComponent->SetSwitchIndex(value);
			entities.insert(en);
		}
	}
}

void SetSwitchIndexHelper::RestoreOriginalIndexes(Map<SwitchComponent *, int32>& originalIndexes, Set<Entity*>& affectedEntities)
{
	for(Map<SwitchComponent *, int32>::iterator it = originalIndexes.begin(); it != originalIndexes.end(); ++it)
	{
		SwitchComponent* component = (*it).first;
		int32 origIndex = (*it).second;
		component->SetSwitchIndex(origIndex);
	}

	originalIndexes.clear();
	affectedEntities.clear();
}

};
