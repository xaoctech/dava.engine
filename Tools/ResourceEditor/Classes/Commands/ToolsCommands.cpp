#include "ToolsCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Main/QtMainWindowHandler.h"


using namespace DAVA;

//Show/Hide Materials
CommandMaterials::CommandMaterials()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandMaterials::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->MaterialsTriggered();
    }
}


//Show/Hide Texture Converter
CommandTextureConverter::CommandTextureConverter()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandTextureConverter::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
		// Replaced with Qt
		// TODO:
		// remove this
		// 
        // screen->TextureConverterTriggered();
    }
}

//Show settings
CommandSettings::CommandSettings()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSettings::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ShowSettings();
    }
}


//Beast
CommandBeast::CommandBeast()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
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


//Ruler Tool
CommandRulerTool::CommandRulerTool()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandRulerTool::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->RulerToolTriggered();
    }
    
    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    activeScene->RebuildSceneGraph();
    
    QtMainWindowHandler::Instance()->ShowStatusBarMessage(activeScene->GetScenePathname().GetAbsolutePathname());
}



CommandConvertToShadow::CommandConvertToShadow()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
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
		}
	}
}
