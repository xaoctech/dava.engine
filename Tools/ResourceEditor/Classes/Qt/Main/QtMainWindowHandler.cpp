#include "QtMainWindowHandler.h"

#include "../Commands/CommandCreateNode.h"
#include "../Commands/CommandsManager.h"
#include "../Commands/FileCommands.h"
#include "../Commands/ToolsCommands.h"
#include "../Commands/CommandViewport.h"
#include "../Commands/SceneGraphCommands.h"
#include "../Commands/ViewCommands.h"
#include "../Commands/CommandReloadTextures.h"
#include "../Commands/ParticleEditorCommands.h"
#include "../Commands/LandscapeOptionsCommands.h"
#include "../Commands/TextureOptionsCommands.h"
#include "../Commands/CustomColorCommands.h"
#include "../Commands/VisibilityCheckToolCommands.h"
#include "../Commands/TilemapEditorCommands.h"
#include "../Commands/HeightmapEditorCommands.h"
#include "../Commands/ModificationOptionsCommands.h"
#include "../Commands/SetSwitchIndexCommands.h"
#include "../Constants.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "GUIState.h"
#include "Scene/SceneDataManager.h"
#include "Scene/SceneData.h"
#include "Main/QtUtils.h"
#include "Main/mainwindow.h"
#include "TextureBrowser/TextureBrowser.h"
#include "Project/ProjectManager.h"
#include "ModificationWidget.h"

#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QStatusBar>
#include <QSpinBox.h>

#include "Render/LibDxtHelper.h"

using namespace DAVA;

QtMainWindowHandler::QtMainWindowHandler(QObject *parent)
    :   QObject(parent)
	,	menuResentScenes(NULL)
    ,   resentAncorAction(NULL)
	,	defaultFocusWidget(NULL)
    ,   statusBar(NULL)
{
    new CommandsManager();
    
    ClearActions(ResourceEditor::NODE_COUNT, nodeActions);
    ClearActions(ResourceEditor::VIEWPORT_COUNT, viewportActions);
    ClearActions(ResourceEditor::HIDABLEWIDGET_COUNT, hidablewidgetActions);
    ClearActions(FILE_FORMAT_COUNT, textureFileFormatActions);
	ClearActions(ResourceEditor::MODIFY_COUNT, modificationActions);

    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        resentSceneActions[i] = new QAction(this);
        resentSceneActions[i]->setObjectName(QString::fromUtf8(Format("resentSceneActions[%d]", i)));
    }
}

QtMainWindowHandler::~QtMainWindowHandler()
{
	defaultFocusWidget = NULL;
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        SafeDelete(resentSceneActions[i]);
    }

    ClearActions(ResourceEditor::NODE_COUNT, nodeActions);
    ClearActions(ResourceEditor::VIEWPORT_COUNT, viewportActions);
    ClearActions(ResourceEditor::HIDABLEWIDGET_COUNT, hidablewidgetActions);
    ClearActions(FILE_FORMAT_COUNT, textureFileFormatActions);
	ClearActions(ResourceEditor::MODIFY_COUNT, modificationActions);

    CommandsManager::Instance()->Release();
}

void QtMainWindowHandler::ClearActions(int32 count, QAction **actions)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = NULL;
    }
}

void QtMainWindowHandler::Execute(Command *command)
{
    CommandsManager::Instance()->Execute(command);
    SafeRelease(command);
}


void QtMainWindowHandler::NewScene()
{
    Execute(new CommandNewScene());
}


void QtMainWindowHandler::OpenScene()
{
    Execute(new CommandOpenScene());
}


void QtMainWindowHandler::OpenProject()
{
	/*
    Execute(new CommandOpenProject());
	emit ProjectChanged();
	*/

	QString newPath = ProjectManager::Instance()->ProjectOpenDialog();
	ProjectManager::Instance()->ProjectOpen(newPath);
}

void QtMainWindowHandler::OpenResentScene(int32 index)
{
    Execute(new CommandOpenScene(EditorSettings::Instance()->GetLastOpenedFile(index)));
}

void QtMainWindowHandler::SaveScene()
{
    Execute(new CommandSaveScene());
}

void QtMainWindowHandler::ExportAsPNG()
{
    Execute(new CommandExport(PNG_FILE));
}

void QtMainWindowHandler::ExportAsPVR()
{
    Execute(new CommandExport(PVR_FILE));
}

void QtMainWindowHandler::ExportAsDXT()
{
    Execute(new CommandExport(DXT_FILE));
}

void QtMainWindowHandler::SaveToFolderWithChilds()
{
    Execute(new CommandSaveToFolderWithChilds());
}

void QtMainWindowHandler::CreateNode(ResourceEditor::eNodeType type)
{
    Execute(new CommandCreateNode(type));
}

void QtMainWindowHandler::Materials()
{
    Execute(new CommandMaterials());

	/*
	MaterialBrowser *materialBrowser = new MaterialBrowser((QWidget *) parent());
	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(NULL != screen)
	{
		SceneEditorScreenMain::BodyItem *body = screen->FindCurrentBody();

		if(NULL != body && NULL != body->bodyControl)
		{
			DAVA::Scene* mainScreenScene = screen->FindCurrentBody()->bodyControl->GetScene();
			materialBrowser->SetScene(mainScreenScene);
		}
	}

	materialBrowser->show();
	*/
}

void QtMainWindowHandler::HeightmapEditor()
{
    Execute(new CommandHeightmapEditor());
}

void QtMainWindowHandler::TilemapEditor()
{
    Execute(new CommandTilemapEditor());
}

void QtMainWindowHandler::ConvertTextures()
{
	TextureBrowser *textureBrowser = new TextureBrowser((QWidget *) parent());
	SceneData *activeScene =  SceneDataManager::Instance()->SceneGetActive();
	
	textureBrowser->sceneActivated(activeScene);
	textureBrowser->sceneNodeSelected(activeScene, SceneDataManager::Instance()->SceneGetSelectedNode(activeScene));
	textureBrowser->show();
}

void QtMainWindowHandler::SetViewport(ResourceEditor::eViewportType type)
{
    Execute(new CommandViewport(type));
}


void QtMainWindowHandler::CreateNodeTriggered(QAction *nodeAction)
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

void QtMainWindowHandler::ViewportTriggered(QAction *viewportAction)
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

void QtMainWindowHandler::SetResentMenu(QMenu *menu)
{
    menuResentScenes = menu;
}

void QtMainWindowHandler::SetResentAncorAction(QAction *ancorAction)
{
    resentAncorAction = ancorAction;
}


void QtMainWindowHandler::MenuFileWillShow()
{
    if(!GUIState::Instance()->GetNeedUpdatedFileMenu()) return;
    
    //TODO: what a bug?
    DVASSERT(menuResentScenes && "Call SetResentMenu() to setup resent menu");

    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        if(resentSceneActions[i]->parentWidget())
        {
            menuResentScenes->removeAction(resentSceneActions[i]);
        }
    }

    
    int32 sceneCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(0 < sceneCount)
    {
        QList<QAction *> resentActions;
        for(int32 i = 0; i < sceneCount; ++i)
        {
            resentSceneActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            resentActions.push_back(resentSceneActions[i]);
        }
        
        menuResentScenes->insertActions(resentAncorAction, resentActions);
        menuResentScenes->insertSeparator(resentAncorAction);
    }
 
    GUIState::Instance()->SetNeedUpdatedFileMenu(false);
}

void QtMainWindowHandler::MenuToolsWillShow()
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


void QtMainWindowHandler::FileMenuTriggered(QAction *resentScene)
{
    for(int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        if(resentScene == resentSceneActions[i])
        {
            OpenResentScene(i);
        }
    }
}


void QtMainWindowHandler::RegisterNodeActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::NODE_COUNT == count) && "Wrong count of actions");

    va_list vl;
    va_start(vl, count);
    
    RegisterActions(nodeActions, count, vl);    
    
    va_end(vl);
}


void QtMainWindowHandler::RegisterViewportActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::VIEWPORT_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(viewportActions, count, vl);    
  
    va_end(vl);
}

void QtMainWindowHandler::RegisterDockActions(int32 count, ...)
{
    DVASSERT((ResourceEditor::HIDABLEWIDGET_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(hidablewidgetActions, count, vl);
    
    va_end(vl);
}

void QtMainWindowHandler::RegisterTextureFormatActions(DAVA::int32 count, ...)
{
    DVASSERT((FILE_FORMAT_COUNT == count) && "Wrong count of actions");
    
    va_list vl;
    va_start(vl, count);
    
    RegisterActions(textureFileFormatActions, count, vl);
    
    va_end(vl);
}

void QtMainWindowHandler::RegisterModificationActions(DAVA::int32 count, ...)
{
	DVASSERT((count == ResourceEditor::MODIFY_COUNT) && "Wrong count of actions");
	va_list vl;
	va_start(vl, count);

	RegisterActions(modificationActions, count, vl);

	va_end(vl);
}



void QtMainWindowHandler::RegisterActions(QAction **actions, int32 count, va_list &vl)
{
    for(int32 i = 0; i < count; ++i)
    {
        actions[i] = va_arg(vl, QAction *);
    }
}


void QtMainWindowHandler::RestoreViews()
{
    for(int32 i = 0; i < ResourceEditor::HIDABLEWIDGET_COUNT; ++i)
    {
        if(hidablewidgetActions[i] && !hidablewidgetActions[i]->isChecked())
        {
            hidablewidgetActions[i]->trigger();
        }
    }
}

void QtMainWindowHandler::RefreshSceneGraph()
{
    Execute(new CommandRefreshSceneGraph());
}

void QtMainWindowHandler::ToggleSceneInfo()
{
    Execute(new CommandSceneInfo());
}

void QtMainWindowHandler::ShowSettings()
{
    Execute(new CommandSettings());
}


void QtMainWindowHandler::Beast()
{
    Execute(new CommandBeast());
}

void QtMainWindowHandler::SetDefaultFocusWidget(QWidget *widget)
{
	defaultFocusWidget = widget;
}

void QtMainWindowHandler::RestoreDefaultFocus()
{
	if(defaultFocusWidget)
	{
		defaultFocusWidget->setEnabled(false);
		defaultFocusWidget->setEnabled(true);
        defaultFocusWidget->setFocus();
	}
}


void QtMainWindowHandler::ReloadTexturesFromFileSystem()
{
    Execute(new CommandReloadTextures());
}

void QtMainWindowHandler::CreateParticleEmitterNode()
{
    CreateNode(ResourceEditor::NODE_PARTICLE_EMITTER);
}

void QtMainWindowHandler::ToggleNotPassableTerrain()
{
	Execute(new CommandNotPassableTerrain());
}

void QtMainWindowHandler::RegisterStatusBar(QStatusBar *registeredSatusBar)
{
    statusBar = registeredSatusBar;
}

void QtMainWindowHandler::ShowStatusBarMessage(const DAVA::String &message, DAVA::int32 displayTime)
{
    if(statusBar)
    {
        statusBar->showMessage(QString(message.c_str()), displayTime);
    }
}


void QtMainWindowHandler::SetWaitingCursorEnabled(bool enabled)
{
    QMainWindow *window = dynamic_cast<QMainWindow *>(parent());
    
    if(window)
    {
        if(enabled)
        {
            window->setCursor(QCursor(Qt::WaitCursor));
        }
        else
        {
            window->setCursor(QCursor(Qt::ArrowCursor));
        }
    }
}

void QtMainWindowHandler::MenuViewOptionsWillShow()
{
    uint8 textureFileFormat = (uint8)EditorSettings::Instance()->GetTextureViewFileFormat();
    
    for(int32 i = 0; i < FILE_FORMAT_COUNT; ++i)
    {
        textureFileFormatActions[i]->setCheckable(true);
        textureFileFormatActions[i]->setChecked(i == textureFileFormat);
    }
}

void QtMainWindowHandler::RulerTool()
{
    Execute(new CommandRulerTool());
}

void QtMainWindowHandler::ReloadAsPNG()
{
    Execute(new ReloadTexturesAsCommand(PNG_FILE));
}
void QtMainWindowHandler::ReloadAsPVR()
{
    Execute(new ReloadTexturesAsCommand(PVR_FILE));
}
void QtMainWindowHandler::ReloadAsDXT()
{
    Execute(new ReloadTexturesAsCommand(DXT_FILE));
}

void QtMainWindowHandler::ToggleSetSwitchIndex(DAVA::uint32  value, SetSwitchIndexHelper::eSET_SWITCH_INDEX state)
{
    Execute(new CommandToggleSetSwitchIndex(value,state));
}

void QtMainWindowHandler::ToggleCustomColors()
{
    Execute(new CommandToggleCustomColors());
}

void QtMainWindowHandler::SaveTextureCustomColors()
{
    Execute(new CommandSaveTextureCustomColors());
}

void QtMainWindowHandler::LoadTextureCustomColors()
{
	Execute(new CommandLoadTextureCustomColors());
}

void QtMainWindowHandler::ChangeBrushSizeCustomColors(int newSize)
{
    Execute(new CommandChangeBrushSizeCustomColors(newSize));
}

void QtMainWindowHandler::ChangeColorCustomColors(int newColorIndex)
{
    Execute(new CommandChangeColorCustomColors(newColorIndex));
}

void QtMainWindowHandler::RegisterCustomColorsWidgets(QPushButton* toggleButton, QPushButton* saveTextureButton, QSlider* brushSizeSlider, QComboBox* colorComboBox, QPushButton* loadTextureButton)
{
	this->customColorsToggleButton = toggleButton;
	this->customColorsSaveTextureButton = saveTextureButton;
	this->customColorsBrushSizeSlider = brushSizeSlider;
	this->customColorsColorComboBox = colorComboBox;
	this->customColorsLoadTextureButton = loadTextureButton;
}

void QtMainWindowHandler::SetCustomColorsWidgetsState(bool state)
{
	DVASSERT(customColorsToggleButton &&
			 customColorsSaveTextureButton &&
			 customColorsBrushSizeSlider &&
			 customColorsColorComboBox &&
			 customColorsLoadTextureButton);

	customColorsToggleButton->blockSignals(true);
	customColorsToggleButton->setCheckable(state);
	customColorsToggleButton->setChecked(state);
	customColorsToggleButton->blockSignals(false);

	QString buttonText = state ? tr("Disable Custom Colors") : tr("Enable Custom Colors");
	customColorsToggleButton->setText(buttonText);

	customColorsSaveTextureButton->setEnabled(state);
	customColorsBrushSizeSlider->setEnabled(state);
	customColorsColorComboBox->setEnabled(state);
	customColorsLoadTextureButton->setEnabled(state);
	customColorsSaveTextureButton->blockSignals(!state);
	customColorsBrushSizeSlider->blockSignals(!state);
	customColorsColorComboBox->blockSignals(!state);
	customColorsLoadTextureButton->blockSignals(!state);

	if(state == true)
	{
		ChangeBrushSizeCustomColors(customColorsBrushSizeSlider->value());
		ChangeColorCustomColors(customColorsColorComboBox->currentIndex());
	}
}

void QtMainWindowHandler::RegisterSetSwitchIndexWidgets(QSpinBox* spinBox, QRadioButton* rBtnSelection, QRadioButton* rBtnScene, QPushButton* btnOK)
{
	this->setSwitchIndexToggleButton = btnOK;
	this->editSwitchIndexValue = spinBox;
	this->rBtnSelection = rBtnSelection;
	this->rBtnScene = rBtnScene;
}

void QtMainWindowHandler::SetSwitchIndexWidgetsState(bool state)
{
	DVASSERT(setSwitchIndexToggleButton &&
		 editSwitchIndexValue &&
		 rBtnSelection &&
		 rBtnScene );

	setSwitchIndexToggleButton->blockSignals(true);
	setSwitchIndexToggleButton->setEnabled(state);
	setSwitchIndexToggleButton->blockSignals(false);

	editSwitchIndexValue->setEnabled(state);
	rBtnSelection->setEnabled(state);
	rBtnScene->setEnabled(state);
}

void QtMainWindowHandler::ToggleVisibilityTool()
{
	Execute(new CommandToggleVisibilityTool());
}

void QtMainWindowHandler::SaveTextureVisibilityTool()
{
	Execute(new CommandSaveTextureVisibilityTool());
}

void QtMainWindowHandler::ChangleAreaSizeVisibilityTool(int newSize)
{
	Execute(new CommandChangeAreaSizeVisibilityTool(newSize));
}

void QtMainWindowHandler::SetVisibilityPointVisibilityTool()
{
	Execute(new CommandSetPointVisibilityTool());
}

void QtMainWindowHandler::SetVisibilityAreaVisibilityTool()
{
	Execute(new CommandSetAreaVisibilityTool());
}

void QtMainWindowHandler::RegisterWidgetsVisibilityTool(QPushButton* toggleButton, QPushButton* saveTextureButton, QPushButton* setPointButton, QPushButton* setAreaButton, QSlider* areaSizeSlider)
{
	visibilityToolToggleButton = toggleButton;
	visibilityToolSaveTextureButton = saveTextureButton;
	visibilityToolSetPointButton = setPointButton;
	visibilityToolSetAreaButton = setAreaButton;
	visibilityToolAreaSizeSlider = areaSizeSlider;
}

void QtMainWindowHandler::SetWidgetsStateVisibilityTool(bool state)
{
	DVASSERT(visibilityToolToggleButton &&
			 visibilityToolSetPointButton &&
			 visibilityToolSetAreaButton &&
			 visibilityToolSaveTextureButton &&
			 visibilityToolAreaSizeSlider);
	
	visibilityToolToggleButton->blockSignals(true);
	visibilityToolToggleButton->setCheckable(state);
	visibilityToolToggleButton->setChecked(state);
	visibilityToolToggleButton->blockSignals(false);
	
	QString toggleButtonText = state ? tr("Disable Visibility Check Tool"): tr("Enable Visibility Check Tool");
	visibilityToolToggleButton->setText(toggleButtonText);

	visibilityToolSaveTextureButton->setEnabled(state);
	visibilityToolSetPointButton->setEnabled(state);
	visibilityToolSetAreaButton->setEnabled(state);
	visibilityToolAreaSizeSlider->setEnabled(state);
	visibilityToolSaveTextureButton->blockSignals(!state);
	visibilityToolSetPointButton->blockSignals(!state);
	visibilityToolSetAreaButton->blockSignals(!state);
	visibilityToolAreaSizeSlider->blockSignals(!state);
	
	if(state == true)
	{
		ChangleAreaSizeVisibilityTool(visibilityToolAreaSizeSlider->value());
	}
}

void QtMainWindowHandler::SetPointButtonStateVisibilityTool(bool state)
{
	DVASSERT(visibilityToolSetPointButton);

	bool b = visibilityToolSetPointButton->signalsBlocked();
	visibilityToolSetPointButton->blockSignals(true);
	visibilityToolSetPointButton->setChecked(state);
	visibilityToolSetPointButton->blockSignals(b);
}

void QtMainWindowHandler::SetAreaButtonStateVisibilityTool(bool state)
{
	DVASSERT(visibilityToolSetAreaButton);

	bool b = visibilityToolSetAreaButton->signalsBlocked();
	visibilityToolSetAreaButton->blockSignals(true);
	visibilityToolSetAreaButton->setChecked(state);
	visibilityToolSetAreaButton->blockSignals(b);
}

void QtMainWindowHandler::ModificationSelect()
{
	SetModificationMode(ResourceEditor::MODIFY_NONE);
	UpdateModificationActions();
}

void QtMainWindowHandler::ModificationMove()
{
	SetModificationMode(ResourceEditor::MODIFY_MOVE);
	UpdateModificationActions();
}

void QtMainWindowHandler::ModificationRotate()
{
	SetModificationMode(ResourceEditor::MODIFY_ROTATE);
	UpdateModificationActions();
}

void QtMainWindowHandler::ModificationScale()
{
	SetModificationMode(ResourceEditor::MODIFY_SCALE);
	UpdateModificationActions();
}

void QtMainWindowHandler::SetModificationMode(ResourceEditor::eModificationActions mode)
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;
	ResourceEditor::eModificationActions curModificationMode = bodyControl->GetModificationMode();

	if (mode != curModificationMode)
		bodyControl->SetModificationMode(mode);
}

void QtMainWindowHandler::ModificationPlaceOnLand()
{
	Execute(new ModificationPlaceOnLandCommand());
}

void QtMainWindowHandler::ModificationSnapToLand()
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;

	bool curSnapToLand = bodyControl->IsLandscapeRelative();
	bodyControl->SetLandscapeRelative(!curSnapToLand);
}

void QtMainWindowHandler::UpdateModificationActions()
{
	SceneEditorScreenMain* screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DVASSERT(screen);

	EditorBodyControl* bodyControl = screen->FindCurrentBody()->bodyControl;
	ResourceEditor::eModificationActions modificationMode = bodyControl->GetModificationMode();

	modificationActions[ResourceEditor::MODIFY_NONE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_MOVE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_ROTATE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_SCALE]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setChecked(false);
	modificationActions[ResourceEditor::MODIFY_NONE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_MOVE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_ROTATE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_SCALE]->setCheckable(true);
	modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setCheckable(true);

	switch (modificationMode)
	{
		case ResourceEditor::MODIFY_MOVE:
		case ResourceEditor::MODIFY_ROTATE:
		case ResourceEditor::MODIFY_SCALE:
			modificationActions[modificationMode]->setChecked(true);
			modificationActions[ResourceEditor::MODIFY_PLACE_ON_LAND]->setEnabled(true);
			modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setEnabled(true);

			if (bodyControl->IsLandscapeRelative())
			{
				modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setChecked(true);
			}
			break;

		case ResourceEditor::MODIFY_NONE:
			modificationActions[modificationMode]->setChecked(true);

		default:
			modificationActions[ResourceEditor::MODIFY_PLACE_ON_LAND]->setEnabled(false);
			modificationActions[ResourceEditor::MODIFY_SNAP_TO_LAND]->setEnabled(false);
			break;
	}
}

void QtMainWindowHandler::OnApplyModification(double x, double y, double z)
{
	Execute(new ModificationApplyCommand(x, y, z));
}

void QtMainWindowHandler::OnResetModification()
{
	Execute(new ModificationResetCommand());
}
