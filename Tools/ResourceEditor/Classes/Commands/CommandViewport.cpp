#include "CommandViewport.h"

#include "DAVAEngine.h"
#include "../SceneEditor/SceneEditorScreenMain.h"

using namespace DAVA;


CommandViewport::CommandViewport(ResourceEditor::eViewportType type)
    :   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
    ,   viewportType(type)
{
}


void CommandViewport::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->SetViewport(viewportType);
    }
}

