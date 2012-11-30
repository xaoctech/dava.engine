#include "LandscapeOptionsCommands.h"

#include "DAVAEngine.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../Qt/Scene/SceneData.h"


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

