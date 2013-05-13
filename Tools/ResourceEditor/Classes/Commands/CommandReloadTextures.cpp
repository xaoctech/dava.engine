#include "CommandReloadTextures.h"

#include "DAVAEngine.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../SceneEditor/EditorSettings.h"

using namespace DAVA;


CommandReloadTextures::CommandReloadTextures()
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT, CommandList::ID_COMMAND_RELOAD_TEXTURES)
{
}


void CommandReloadTextures::Execute()
{
    SceneDataManager::Instance()->TextureReloadAll(EditorSettings::Instance()->GetTextureViewGPU());
}

