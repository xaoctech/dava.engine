#include "HangingObjectsHelper.h"
#include "../Qt/Scene/SceneDataManager.h"

namespace DAVA
{

void HangingObjectsHelper::ProcessHangingObjectsUpdate(float value)
{
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		
		List<Entity*> renderComponents;
		
		sceneData->GetScene()->FindComponentsByTypeRecursive(Component::RENDER_COMPONENT, renderComponents);
		
		for(List<Entity*>::const_iterator it = renderComponents.begin(); it != renderComponents.end(); ++it)
		{
			RenderComponent * renderComponent = cast_if_equal<RenderComponent*>((*it)->GetComponent(Component::RENDER_COMPONENT));
			if(NULL == renderComponent)
			{
				continue;
			}

			RenderObject* renderObject = renderComponent->GetRenderObject();
			if(RenderObject::TYPE_MESH == renderObject->GetType())
			{
				
			}
		}
	}
}

};
