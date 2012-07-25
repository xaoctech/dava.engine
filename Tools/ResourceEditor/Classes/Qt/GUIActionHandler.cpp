#include "GUIActionHandler.h"

#include "../Commands/CommandCreateNode.h"
#include "../Commands/CommandsManager.h"
#include "../Commands/FileCommands.h"
#include "../Commands/ToolsCommands.h"
#include "../Commands/CommandViewport.h"
#include "../Constants.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "GUIState.h"

using namespace DAVA;

GUIActionHandler::GUIActionHandler(QObject *parent)
    :   QObject(parent)
{
    new CommandsManager();
    
    for(int32 i = 0; i < ResourceEditor::NODE_COUNT; ++i)
    {
        nodeActions[i] = NULL;
    }
    
    for(int32 i = 0; i < ResourceEditor::VIEWPORT_COUNT; ++i)
    {
        viewportActions[i] = NULL;
    }

    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        resentSceneActions[i] = new QAction(this);
        resentSceneActions[i]->setObjectName(QString::fromUtf8(Format("resentSceneActions[%d]", i)));
    }
}

GUIActionHandler::~GUIActionHandler()
{
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        SafeDelete(resentSceneActions[i]);
    }

    
    for(int32 i = 0; i < ResourceEditor::NODE_COUNT; ++i)
    {
        nodeActions[i] = NULL;
    }
    
    for(int32 i = 0; i < ResourceEditor::VIEWPORT_COUNT; ++i)
    {
        viewportActions[i] = NULL;
    }

    
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

void GUIActionHandler::OpenResentScene(int32 index)
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

void GUIActionHandler::CreateNodeTriggered(QAction *nodeAction)
{
    for(int32 i = 0; i < ResourceEditor::NODE_COUNT; ++i)
    {
        if(nodeAction == nodeActions[i])
        {
            CreateNode((ResourceEditor::eNodeType)i);
            break;
        }
    }
}

void GUIActionHandler::ViewportTriggered(QAction *viewportAction)
{
    for(int32 i = 0; i < ResourceEditor::VIEWPORT_COUNT; ++i)
    {
        if(viewportAction == viewportActions[i])
        {
            SetViewport((ResourceEditor::eViewportType)i);
            break;
        }
    }
}

void GUIActionHandler::SetResentMenu(QMenu *menu)
{
    menuResentScenes = menu;
}

void GUIActionHandler::MenuFileWillShow()
{
    if(!GUIState::Instance()->GetNeedUpdatedFileMenu()) return;
    
    DVASSERT(menuResentScenes && "Call SetResentMenu() to setup resent menu");
    
    int32 sceneCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(0 < sceneCount)
    {
        menuResentScenes->setEnabled(true);
        
        for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
        {
            if(resentSceneActions[i]->parentWidget())
            {
                menuResentScenes->removeAction(resentSceneActions[i]);
            }
        }
        
        for(int32 i = 0; i < sceneCount; ++i)
        {
            resentSceneActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            menuResentScenes->addAction(resentSceneActions[i]);
        }
    }
    else 
    {
        menuResentScenes->setEnabled(false);
    }
 
    GUIState::Instance()->SetNeedUpdatedFileMenu(false);
}

void GUIActionHandler::MenuToolsWillShow()
{
    if(!GUIState::Instance()->GetNeedUpdatedToolsMenu()) return;
    
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
//        screen->;
    }
    
    GUIState::Instance()->SetNeedUpdatedToolsMenu(false);
}


void GUIActionHandler::ResentSceneTriggered(QAction *resentScene)
{
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        if(resentScene == resentSceneActions[i])
        {
            OpenResentScene(i);
        }
    }
}


void GUIActionHandler::RegisterNodeActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::NODE_COUNT == count) && "Wrong count of actions");

    va_list vl;
    va_start(vl, count);
    
    RegisterActions(nodeActions, count, vl);    
    
    va_end(vl);
}


void GUIActionHandler::RegisterViewportActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::VIEWPORT_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(viewportActions, count, vl);    
  
    va_end(vl);
}

void GUIActionHandler::RegisterActions(QAction **actions, int32 count, va_list &vl)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = va_arg(vl, QAction *);
    }
}


