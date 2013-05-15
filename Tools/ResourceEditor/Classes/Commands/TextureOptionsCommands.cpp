#include "TextureOptionsCommands.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneValidator.h"
#include "../Qt/Scene/SceneDataManager.h"

using namespace DAVA;

ReloadTexturesAsCommand::ReloadTexturesAsCommand(eGPUFamily gpu)
    :   Command(COMMAND_CLEAR_UNDO_QUEUE, CommandList::ID_COMMAND_RELOAD_TEXTURES_AS)
    ,   gpuFamily(gpu)
{
}

void ReloadTexturesAsCommand::Execute()
{
    Texture::SetDefaultGPU(gpuFamily);
    
    EditorSettings::Instance()->SetTextureViewGPU(gpuFamily);
    EditorSettings::Instance()->Save();
    
    SceneDataManager::Instance()->TextureReloadAll(gpuFamily);
//    SceneValidator::Instance()->EnumerateSceneTextures();
}

