#include "CommandReloadTextures.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneValidator.h"
#include "../SceneEditor/EditorSettings.h"

using namespace DAVA;


CommandReloadTextures::CommandReloadTextures()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandReloadTextures::Execute()
{
    SceneValidator::Instance()->ReloadTextures((ImageFileFormat)EditorSettings::Instance()->GetTextureViewFileFormat());
}

