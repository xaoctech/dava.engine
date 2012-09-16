#include "LandscapeOptionsCommands.h"

#include "DAVAEngine.h"
#include "../Qt/SceneDataManager.h"
#include "../Qt/SceneData.h"


using namespace DAVA;

CommandNotPassableTerrain::CommandNotPassableTerrain()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandNotPassableTerrain::Execute()
{
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->ToggleNotPassableLandscape();
}



CommandGriddableLandscape::CommandGriddableLandscape()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandGriddableLandscape::Execute()
{
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->ToggleGriddableLandscape();
}

