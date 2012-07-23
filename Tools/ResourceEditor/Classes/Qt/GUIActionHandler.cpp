#include "GUIActionHandler.h"

#include "../Commands/CommandsManager.h"
#include "../Commands/FileCommands.h"
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
    CommandNewScene *command = new CommandNewScene();
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}


void GUIActionHandler::OpenScene()
{
    CommandOpenScene *command = new CommandOpenScene();
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}


void GUIActionHandler::OpenProject()
{
    CommandOpenProject *command = new CommandOpenProject();
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}

void GUIActionHandler::OpenResentScene(DAVA::int32 index)
{
    CommandOpenScene *command = new CommandOpenScene(EditorSettings::Instance()->GetLastOpenedFile(index));
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}

void GUIActionHandler::SaveScene()
{
    CommandSaveScene *command = new CommandSaveScene();
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}

void GUIActionHandler::ExportAsPNG()
{
    CommandExport *command = new CommandExport(ResourceEditor::FORMAT_PNG);
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);

}
void GUIActionHandler::ExportAsPVR()
{
    CommandExport *command = new CommandExport(ResourceEditor::FORMAT_PVR);
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}

void GUIActionHandler::ExportAsDXT()
{
    CommandExport *command = new CommandExport(ResourceEditor::FORMAT_DXT);
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}

void GUIActionHandler::Materials()
{
    
}

void GUIActionHandler::CreateLandscape()
{
    
}

void GUIActionHandler::CreateLight()
{
    
}

void GUIActionHandler::CreateServiceNode()
{
    
}

void GUIActionHandler::CreateBox()
{
    
}

void GUIActionHandler::CreateSphere()
{
    
}

void GUIActionHandler::CreateCamera()
{
    
}

void GUIActionHandler::CreateImposter()
{
    
}

void GUIActionHandler::CreateUserNode()
{
    
}


void GUIActionHandler::HeightmapEditor()
{
    
}

void GUIActionHandler::TilemapEditor()
{
    
}

void GUIActionHandler::ViewportiPhone()
{
    
}

void GUIActionHandler::VeiwportRetina()
{
    
}

void GUIActionHandler::ViewportiPad()
{
    
}

void GUIActionHandler::ViewportDefault()
{
    
}

void GUIActionHandler::ConvertTextures()
{
    
}

