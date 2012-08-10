#include "GUIActionHandler.h"

#include "../Commands/CommandCreateNode.h"
#include "../Commands/CommandsManager.h"
#include "../Commands/FileCommands.h"
#include "../Commands/ToolsCommands.h"
#include "../Commands/CommandViewport.h"
#include "../Commands/SceneGraphCommands.h"
#include "../Commands/LibraryCommands.h"
#include "../Commands/ViewCommands.h"
#include "../Constants.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "GUIState.h"
#include "SceneDataManager.h"
#include "SceneData.h"
#include "QtUtils.h"

#include <QPoint>
#include <QTreeView>
#include <QMenu>
#include <QAction>
#include <QCursor>

using namespace DAVA;

GUIActionHandler* GUIActionHandler::activeActionHandler = NULL;

GUIActionHandler::GUIActionHandler(QObject *parent)
    :   QObject(parent)
{
    activeActionHandler = this;
    
    
    new CommandsManager();
    
    ClearActions(ResourceEditor::NODE_COUNT, nodeActions);
    ClearActions(ResourceEditor::VIEWPORT_COUNT, viewportActions);
    ClearActions(ResourceEditor::HIDABLEWIDGET_COUNT, hidablewidgetActions);

    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        resentSceneActions[i] = new QAction(this);
        resentSceneActions[i]->setObjectName(QString::fromUtf8(Format("resentSceneActions[%d]", i)));
    }
    
    menuResentScenes = NULL;
    
    libraryView = NULL;
}

GUIActionHandler::~GUIActionHandler()
{
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        SafeDelete(resentSceneActions[i]);
    }

    ClearActions(ResourceEditor::NODE_COUNT, nodeActions);
    ClearActions(ResourceEditor::VIEWPORT_COUNT, viewportActions);
    ClearActions(ResourceEditor::HIDABLEWIDGET_COUNT, hidablewidgetActions);
    
    CommandsManager::Instance()->Release();
}

GUIActionHandler * GUIActionHandler::Instance()
{
    return activeActionHandler;
}

void GUIActionHandler::ClearActions(int32 count, QAction **actions)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = NULL;
    }
}

void GUIActionHandler::Execute(Command *command)
{
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
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
    
    //TODO: what a bug?
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

    //TODO: need code here

//    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
//    if(screen)
//    {
////        screen->;
//    }
    
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

void GUIActionHandler::RegisterDockActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::HIDABLEWIDGET_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(hidablewidgetActions, count, vl);
    
    va_end(vl);
}


void GUIActionHandler::RegisterActions(QAction **actions, int32 count, va_list &vl)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = va_arg(vl, QAction *);
    }
}


void GUIActionHandler::RestoreViews()
{
    for(int32 i = 0; i < ResourceEditor::HIDABLEWIDGET_COUNT; ++i)
    {
        if(hidablewidgetActions[i] && !hidablewidgetActions[i]->isChecked())
        {
            hidablewidgetActions[i]->trigger();
        }
    }
}

void GUIActionHandler::RemoveRootNodes()
{
    Execute(new CommandRemoveRootNodes());
}

void GUIActionHandler::RefreshSceneGraph()
{
    Execute(new CommandRefreshSceneGraph());
}

void GUIActionHandler::LockAtObject()
{
    Execute(new CommandLockAtObject());
}

void GUIActionHandler::RemoveObject()
{
    Execute(new CommandRemoveSceneNode());
}

void GUIActionHandler::DebugFlags()
{
    Execute(new CommandDebugFlags());
}

void GUIActionHandler::BakeMatrixes()
{
    Execute(new CommandBakeMatrices());
}

void GUIActionHandler::BuildQuadTree()
{
    Execute(new CommandBuildQuadTree());
}

void GUIActionHandler::SetLibraryView(QTreeView *view)
{
    libraryView = view;
}

void GUIActionHandler::LibraryContextMenuRequested(const QPoint &point)
{
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    
    QModelIndex itemIndex = libraryView->indexAt(point);
    activeScene->ShowLibraryMenu(itemIndex, QCursor::pos());
}

void GUIActionHandler::LibraryMenuTriggered(QAction *fileAction)
{
    String filePathname = QSTRING_TO_DAVASTRING(fileAction->data().toString());
    
    QString actionName = fileAction->text();
    if(QString("Add") == actionName)
    {
        Execute(new CommandAddScene(filePathname));
    }
    else if(QString("Edit") == actionName)
    {
        Execute(new CommandEditScene(filePathname));
    }
    else if(QString("Reload") == actionName)
    {
        Execute(new CommandReloadScene(filePathname));
    }
    else if(QString("Convert") == actionName)
    {
        Execute(new CommandConvertScene(filePathname));
    }
    else
    {
        DVASSERT(0 && "Wrong action");
    }
}

void GUIActionHandler::FileSelected(const QString &filePathname, bool isFile)
{
    //TODO: need best way to display scene preview
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        String extension = FileSystem::Instance()->GetExtension(QSTRING_TO_DAVASTRING(filePathname));
        if(0 == CompareStrings(extension, String(".sc2")) && isFile)
        {
            screen->ShowScenePreview(QSTRING_TO_DAVASTRING(filePathname));
        }
        else
        {
            screen->HideScenePreview();
        }
    }
}


void GUIActionHandler::ToggleSceneInfo()
{
    Execute(new CommandSceneInfo());
}

void GUIActionHandler::ShowSettings()
{
    Execute(new CommandSettings());
}

void GUIActionHandler::BakeScene()
{
    Execute(new CommandBakeScene());
}


