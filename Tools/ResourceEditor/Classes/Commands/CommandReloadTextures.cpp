#include "CommandReloadTextures.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/SceneValidator.h"

using namespace DAVA;


CommandReloadTextures::CommandReloadTextures()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandReloadTextures::Execute()
{
    SceneValidator::Instance()->ReloadTextures();

    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->RecreteFullTilingTexture();
    }
}

