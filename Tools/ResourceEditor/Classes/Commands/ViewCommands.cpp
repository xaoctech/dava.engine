#include "ViewCommands.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"


using namespace DAVA;

//Open Project
CommandSceneInfo::CommandSceneInfo()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandSceneInfo::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->ToggleSceneInfo();
    }
}


