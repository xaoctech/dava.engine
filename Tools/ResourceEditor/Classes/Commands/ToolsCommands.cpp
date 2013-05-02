#include "ToolsCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtMainWindowHandler.h"


using namespace DAVA;


//Beast
CommandBeast::CommandBeast()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_BEAST)
{
}


void CommandBeast::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ProcessBeast();
    }
}


CommandConvertToShadow::CommandConvertToShadow()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_CONVERT_TO_SHADOW)
{

}

void CommandConvertToShadow::Execute()
{
	Entity * entity = SceneDataManager::Instance()->SceneGetSelectedNode(SceneDataManager::Instance()->SceneGetActive());
	if(entity)
	{
		RenderComponent * rc = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
		RenderObject * ro = GetRenerObject(entity);
		if(ro->GetRenderBatchCount() == 1 && typeid(*(ro->GetRenderBatch(0))) == typeid(DAVA::RenderBatch))
		{
			SceneDataManager::Instance()->SceneGetActive()->SelectNode(0);

			ShadowVolume * shadowVolume = ro->CreateShadow();

			ro->RemoveRenderBatch(ro->GetRenderBatch(0));
			ro->AddRenderBatch(shadowVolume);

			entity->SetLocalTransform(entity->GetLocalTransform());//just forced update of worldTransform

			
			SceneDataManager::Instance()->SceneGetActive()->SelectNode(entity);

			affectedEntity = entity;
		}
	}
}

DAVA::Set<DAVA::Entity*> CommandConvertToShadow::GetAffectedEntities()
{
	Set<Entity*> entities;
	entities.insert(affectedEntity);

	return entities;
}
