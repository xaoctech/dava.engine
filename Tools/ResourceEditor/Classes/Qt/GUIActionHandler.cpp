#include "GUIActionHandler.h"

#include "../Commands/CommandCreateNode.h"
#include "../Commands/CommandsManager.h"
#include "../Commands/FileCommands.h"
#include "../Commands/ToolsCommands.h"
#include "../Commands/CommandViewport.h"
#include "../Constants.h"
#include "../SceneEditor/EditorSettings.h"

using namespace DAVA;

GUIActionHandler::GUIActionHandler(QObject *parent)
    :   QObject(parent)
{
    new CommandsManager();
}

GUIActionHandler::~GUIActionHandler()
{
    CommandsManager::Instance()->Release();
}


void GUIActionHandler::NewScene()
{
    Execute(new CommandNewScene());
}


void GUIActionHandler::OpenScene()
{
    Execute(new CommandOpenScene());
}


void GUIActionHandler::OpenProject()
{
    Execute(new CommandOpenProject());
}

void GUIActionHandler::OpenResentScene(DAVA::int32 index)
{
    Execute(new CommandOpenScene(EditorSettings::Instance()->GetLastOpenedFile(index)));
}

void GUIActionHandler::SaveScene()
{
    Execute(new CommandSaveScene());
}

void GUIActionHandler::ExportAsPNG()
{
    Execute(new CommandExport(ResourceEditor::FORMAT_PNG));
}
void GUIActionHandler::ExportAsPVR()
{
    Execute(new CommandExport(ResourceEditor::FORMAT_PVR));
}

void GUIActionHandler::ExportAsDXT()
{
    Execute(new CommandExport(ResourceEditor::FORMAT_DXT));
}

void GUIActionHandler::CreateNode(ResourceEditor::eNodeType type)
{
    Execute(new CommandCreateNode(type));
}

void GUIActionHandler::Materials()
{
    Execute(new CommandMaterials());
}

void GUIActionHandler::HeightmapEditor()
{
    Execute(new CommandHeightmapEditor());
}

void GUIActionHandler::TilemapEditor()
{
    Execute(new CommandTilemapEditor());
}

void GUIActionHandler::ConvertTextures()
{
    Execute(new CommandTextureConverter());
}

void GUIActionHandler::SetViewport(ResourceEditor::eViewportType type)
{
    Execute(new CommandViewport(type));
}

void GUIActionHandler::Execute(Command *command)
{
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}
