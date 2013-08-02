/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DAVAEngine.h"

#include "mainwindow.h"
#include "QtUtils.h"
#include "Project/ProjectManager.h"
#include "DockConsole/Console.h"
#include "Scene/SceneHelper.h"
#include "SpritesPacker/SpritePackerHelper.h"

#include "TextureBrowser/TextureBrowser.h"
#include "MaterialBrowser/MaterialBrowser.h"

#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/SceneEditor/EditorConfig.h"

#include "../CubemapEditor/CubemapTextureBrowser.h"

#include <QFileDialog>
#include <QMessageBox>

QtMainWindow::QtMainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, waitDialog(NULL)
{
	Console::Instance();
	new ProjectManager();

	ui->setupUi(this);

	qApp->installEventFilter(this);
	EditorConfig::Instance()->ParseConfig(EditorSettings::Instance()->GetProjectPath() + "EditorConfig.yaml");

	SetupMainMenu();
	SetupToolBars();
	SetupDocks();
	SetupActions();

	// create tool windows
	new TextureBrowser(this);
	materialEditor = new MaterialEditor(DAVA::Rect(20, 20, 500, 600));
	waitDialog = new QtWaitDialog(this);

	// initial state is as project closed
	ProjectClosed();

	posSaver.Attach(this);
	posSaver.LoadState(this);

	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed()), this, SLOT(ProjectClosed()));

	QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(SceneCommandExecuted(SceneEditor2 *, const Command2*, bool)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));

	QObject::connect(SceneSignals::Instance(), SIGNAL(RulerToolLengthChanged(SceneEditor2*, double, double)), this, SLOT(UpdateRulerToolLength(SceneEditor2*, double, double)));

	LoadGPUFormat();
}

QtMainWindow::~QtMainWindow()
{
	materialEditor->Release();
	TextureBrowser::Instance()->Release();

	posSaver.SaveState(this);

	delete ui;
	ui = NULL;

	ProjectManager::Instance()->Release();
}

Ui::MainWindow* QtMainWindow::GetUI()
{
	return ui;
}

SceneTabWidget* QtMainWindow::GetSceneWidget()
{
	return ui->sceneTabWidget;
}

SceneEditor2* QtMainWindow::GetCurrentScene()
{
	return ui->sceneTabWidget->GetCurrentScene();
}

bool QtMainWindow::SaveSceneAs(SceneEditor2 *scene)
{
	bool ret = false;

	if(NULL != scene)
	{
		DAVA::FilePath saveAsPath = DAVA::FilePath(ProjectManager::Instance()->CurProjectDataSourcePath().toStdString()) + scene->GetScenePath().GetFilename();

		QString selectedPath = QFileDialog::getSaveFileName(this, "Save scene as", saveAsPath.GetAbsolutePathname().c_str(), "DAVA Scene V2 (*.sc2)");
		if(!selectedPath.isEmpty())
		{
			DAVA::FilePath scenePath = DAVA::FilePath(selectedPath.toStdString());
			if(!scenePath.IsEmpty())
			{
				scene->SetScenePath(scenePath);
				ret = scene->Save(scenePath);

				if(!ret)
				{
					QMessageBox::warning(this, "Save error", "An error occurred while saving the scene. Please, see logs for more info.", QMessageBox::Ok);
				}
				else
				{
					AddRecent(scenePath.GetAbsolutePathname().c_str());
				}
			}
		}
	}

	return ret;
}

DAVA::eGPUFamily QtMainWindow::GetGPUFormat()
{
	return EditorSettings::Instance()->GetTextureViewGPU();
}

void QtMainWindow::SetGPUFormat(DAVA::eGPUFamily gpu)
{
	EditorSettings::Instance()->SetTextureViewGPU(gpu);
	DAVA::Texture::SetDefaultGPU(gpu);

	DAVA::Map<DAVA::String, DAVA::Texture *> allScenesTextures;
	for(int tab = 0; tab < GetSceneWidget()->GetTabCount(); ++tab)
	{
		SceneEditor2 *scene = GetSceneWidget()->GetTabScene(tab);
		SceneHelper::EnumerateTextures(scene, allScenesTextures);
	}

	if(allScenesTextures.size() > 0)
	{
		int progress = 0;
		WaitStart("Textures reload", "", 0, allScenesTextures.size());

		DAVA::Map<DAVA::String, DAVA::Texture *>::const_iterator it = allScenesTextures.begin();
		DAVA::Map<DAVA::String, DAVA::Texture *>::const_iterator end = allScenesTextures.end();

		for(; it != end; ++it)
		{
			it->second->ReloadAs(gpu);

			WaitSetMessage(it->first.c_str());
			WaitSetValue(progress++);
		}

		WaitStop();
	}

	LoadGPUFormat();
}

void QtMainWindow::WaitStart(const QString &title, const QString &message, int min /* = 0 */, int max /* = 100 */)
{
	waitDialog->SetRange(min, max);
	waitDialog->Exec(message, title, false, false);
}

void QtMainWindow::WaitSetMessage(const QString &messsage)
{
	waitDialog->SetMessage(messsage);
}

void QtMainWindow::WaitSetValue(int value)
{
	waitDialog->SetValue(value);
}

void QtMainWindow::WaitStop()
{
	waitDialog->Reset();
}

bool QtMainWindow::eventFilter(QObject *obj, QEvent *event)
{
	QEvent::Type eventType = event->type();

	if(qApp == obj && ProjectManager::Instance()->IsOpened())
	{
		if(QEvent::ApplicationActivate == eventType)
		{
			if(QtLayer::Instance())
			{
				QtLayer::Instance()->OnResume();
				Core::Instance()->GetApplicationCore()->OnResume();
			}
		}
		else if(QEvent::ApplicationDeactivate == eventType)
		{
			if(QtLayer::Instance())
			{
				QtLayer::Instance()->OnSuspend();
				Core::Instance()->GetApplicationCore()->OnSuspend();
			}
		}
	}

	if(obj == this && QEvent::WindowUnblocked == eventType)
	{
		if(isActiveWindow())
		{
			ui->sceneTabWidget->setFocus(Qt::ActiveWindowFocusReason);
		}
	}

	return QMainWindow::eventFilter(obj, event);
}

void QtMainWindow::SetupTitle()
{
	DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
	QString title = options->GetString("title").c_str();

	if(ProjectManager::Instance()->IsOpened())
	{
		title += " | Project - ";
		title += ProjectManager::Instance()->CurProjectPath();
	}

	this->setWindowTitle(title);
}

void QtMainWindow::SetupMainMenu()
{
	QAction *actionProperties = ui->dockProperties->toggleViewAction();
	QAction *actionLibrary = ui->dockLibrary->toggleViewAction();
	QAction *actionHangingObjects = ui->dockHangingObjects->toggleViewAction();
	QAction *actionSetSwitchIndex = ui->dockSetSwitchIndex->toggleViewAction();
	QAction *actionParticleEditor = ui->dockParticleEditor->toggleViewAction();
	QAction *actionParticleEditorTimeLine = ui->dockParticleEditorTimeLine->toggleViewAction();
	QAction *actionSceneInfo = ui->dockSceneInfo->toggleViewAction();
	QAction *actionSceneTree = ui->dockSceneTree->toggleViewAction();
	QAction *actionConsole = ui->dockConsole->toggleViewAction();
	QAction *actionCustomColors2 = ui->dockCustomColorsEditor->toggleViewAction();
	QAction *actionVisibilityTool2 = ui->dockVisibilityToolEditor->toggleViewAction();
	QAction *actionHeightmapEditor2 = ui->dockHeightmapEditor->toggleViewAction();
	QAction *actionTilemaskEditor2 = ui->dockTilemaskEditor->toggleViewAction();

	ui->menuView->addAction(actionSceneInfo);
	ui->menuView->addAction(actionLibrary);
	ui->menuView->addAction(actionProperties);
	ui->menuView->addAction(actionParticleEditor);
	ui->menuView->addAction(actionParticleEditorTimeLine);
	ui->menuView->addAction(actionHangingObjects);
	ui->menuView->addAction(actionSetSwitchIndex);
	ui->menuView->addAction(actionSceneTree);
	ui->menuView->addAction(actionConsole);
	ui->menuView->addAction(actionCustomColors2);
	ui->menuView->addAction(actionVisibilityTool2);
	ui->menuView->addAction(actionHeightmapEditor2);
	ui->menuView->addAction(actionTilemaskEditor2);

	InitRecent();
}

void QtMainWindow::SetupToolBars()
{
	QAction *actionMainToolBar = ui->mainToolBar->toggleViewAction();
	QAction *actionModifToolBar = ui->modificationToolBar->toggleViewAction();
	QAction *actionViewModeToolBar = ui->viewModeToolBar->toggleViewAction();

	ui->menuToolbars->addAction(actionMainToolBar);
	ui->menuToolbars->addAction(actionModifToolBar);
	ui->menuToolbars->addAction(actionViewModeToolBar);

	modificationWidget = new ModificationWidget(NULL);
	ui->modificationToolBar->insertWidget(ui->actionModifyReset, modificationWidget);

	// reload
	QToolButton *reloadTexturesBtn = new QToolButton();
	reloadTexturesBtn->setMenu(ui->menuTexturesForGPU);
	reloadTexturesBtn->setPopupMode(QToolButton::MenuButtonPopup);
	reloadTexturesBtn->setDefaultAction(ui->actionReloadTextures);
	reloadTexturesBtn->setMaximumWidth(100);
	reloadTexturesBtn->setMinimumWidth(100);
	ui->mainToolBar->addSeparator();
	ui->mainToolBar->addWidget(reloadTexturesBtn);
	reloadTexturesBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	reloadTexturesBtn->setAutoRaise(false);

	/*
	QAction *reloadMenuAction = ui->menuReload->menuAction();
	reloadMenuAction->setIcon(QIcon(":/QtIcons/reloadtextures.png"));
	ui->mainToolBar->addAction(reloadMenuAction);
	ShowActionWithText(ui->mainToolBar, reloadMenuAction, true);
	*/
}

void QtMainWindow::SetupDocks()
{
	QObject::connect(ui->sceneTreeFilterClear, SIGNAL(pressed()), ui->sceneTreeFilterEdit, SLOT(clear()));
	QObject::connect(ui->sceneTreeFilterEdit, SIGNAL(textChanged(const QString &)), ui->sceneTree, SLOT(SetFilter(const QString &)));
}

void QtMainWindow::SetupActions()
{
	// scene file actions
	QObject::connect(ui->actionOpenProject, SIGNAL(triggered()), this, SLOT(OnProjectOpen()));
	QObject::connect(ui->actionOpenScene, SIGNAL(triggered()), this, SLOT(OnSceneOpen()));
	QObject::connect(ui->actionNewScene, SIGNAL(triggered()), this, SLOT(OnSceneNew()));
	QObject::connect(ui->actionSaveScene, SIGNAL(triggered()), this, SLOT(OnSceneSave()));
	QObject::connect(ui->actionSaveSceneAs, SIGNAL(triggered()), this, SLOT(OnSceneSaveAs()));
	QObject::connect(ui->actionSaveToFolder, SIGNAL(triggered()), this, SLOT(OnSceneSaveToFolder()));

	// export
	QObject::connect(ui->menuExport, SIGNAL(triggered(QAction *)), this, SLOT(ExportMenuTriggered(QAction *)));
	
	// reload
	ui->actionReloadPoverVRIOS->setData(GPU_POWERVR_IOS);
	ui->actionReloadPoverVRAndroid->setData(GPU_POWERVR_ANDROID);
	ui->actionReloadTegra->setData(GPU_TEGRA);
	ui->actionReloadMali->setData(GPU_MALI);
	ui->actionReloadAdreno->setData(GPU_ADRENO);
	ui->actionReloadPNG->setData(GPU_UNKNOWN);
	QObject::connect(ui->menuTexturesForGPU, SIGNAL(triggered(QAction *)), this, SLOT(OnReloadTexturesTriggered(QAction *)));
	QObject::connect(ui->actionReloadTextures, SIGNAL(triggered()), this, SLOT(OnReloadTextures()));
	
	// scene undo/redo
	QObject::connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(OnUndo()));
	QObject::connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(OnRedo()));

	// scene modifications
	QObject::connect(ui->actionModifySelect, SIGNAL(triggered()), this, SLOT(OnSelectMode()));
	QObject::connect(ui->actionModifyMove, SIGNAL(triggered()), this, SLOT(OnMoveMode()));
	QObject::connect(ui->actionModifyRotate, SIGNAL(triggered()), this, SLOT(OnRotateMode()));
	QObject::connect(ui->actionModifyScale, SIGNAL(triggered()), this, SLOT(OnScaleMode()));
	QObject::connect(ui->actionPivotCenter, SIGNAL(triggered()), this, SLOT(OnPivotCenterMode()));
	QObject::connect(ui->actionPivotCommon, SIGNAL(triggered()), this, SLOT(OnPivotCommonMode()));
	QObject::connect(ui->actionManualModifMode, SIGNAL(triggered()), this, SLOT(OnManualModifMode()));
	QObject::connect(ui->actionModifyPlaceOnLandscape, SIGNAL(triggered()), this, SLOT(OnPlaceOnLandscape()));
	QObject::connect(ui->actionModifySnapToLandscape, SIGNAL(triggered()), this, SLOT(OnSnapToLandscape()));

	// tools
	QObject::connect(ui->actionMaterialEditor, SIGNAL(triggered()), this, SLOT(OnMaterialEditor()));
	QObject::connect(ui->actionTextureConverter, SIGNAL(triggered()), this, SLOT(OnTextureBrowser()));
	QObject::connect(ui->actionEnableCameraLight, SIGNAL(triggered()), this, SLOT(OnSceneLightMode()));
	QObject::connect(ui->actionCubemapEditor, SIGNAL(triggered()), this, SLOT(OnCubemapEditor()));
	QObject::connect(ui->actionShowNotPassableLandscape, SIGNAL(triggered()), this, SLOT(OnNotPassableTerrain()));
	QObject::connect(ui->actionRulerTool, SIGNAL(triggered()), this, SLOT(OnRulerTool()));
}

void QtMainWindow::InitRecent()
{
	for(int i = 0; i < EditorSettings::Instance()->GetLastOpenedCount(); ++i)
	{
		DAVA::String path = EditorSettings::Instance()->GetLastOpenedFile(i);
		QAction *action = ui->menuFile->addAction(path.c_str());

		action->setData(QString(path.c_str()));
		recentScenes.push_back(action);
	}

	QObject::connect(ui->menuFile, SIGNAL(triggered(QAction *)), this, SLOT(OnRecentTriggered(QAction *)));
}

void QtMainWindow::AddRecent(const QString &path)
{
	for(int i = 0; i < recentScenes.size(); ++i)
	{
		if(recentScenes[i]->data() == path)
		{
			ui->menuFile->removeAction(recentScenes[i]);
			recentScenes.removeAt(i);
			i--;
		}
	}

	QAction *action = new QAction(path, NULL);
	action->setData(path);

	if(recentScenes.size() > 0)
	{
		ui->menuFile->insertAction(recentScenes[0], action);
	}
	else
	{
		ui->menuFile->addAction(action);
	}

	recentScenes.push_front(action);

	EditorSettings::Instance()->AddLastOpenedFile(DAVA::FilePath(path.toStdString()));
}

// ###################################################################################################
// Scene signals
// ###################################################################################################

void QtMainWindow::ProjectOpened(const QString &path)
{
	ui->actionNewScene->setEnabled(true);
	ui->actionOpenScene->setEnabled(true);
	ui->actionSaveScene->setEnabled(true);
	ui->actionSaveToFolder->setEnabled(true);

	SetupTitle();
}

void QtMainWindow::ProjectClosed()
{
	ui->actionNewScene->setEnabled(false);
	ui->actionOpenScene->setEnabled(false);
	ui->actionSaveScene->setEnabled(false);
	ui->actionSaveToFolder->setEnabled(false);

	SetupTitle();
}

void QtMainWindow::SceneActivated(SceneEditor2 *scene)
{
	LoadUndoRedoState(scene);
	LoadModificationState(scene);
	LoadEditorLightState(scene);
	LoadNotPassableState(scene);
	LoadRulerToolState(scene);

	// TODO: remove this code. it is for old material editor -->
	DAVA::UIControl* parent = materialEditor->GetParent();
	if(NULL != parent && NULL != scene)
	{
		parent->RemoveControl(materialEditor);
		materialEditor->SetWorkingScene(scene, scene->selectionSystem->GetSelection()->GetEntity(0));
		parent->AddControl(materialEditor);
	}
	// <---
}

void QtMainWindow::SceneDeactivated(SceneEditor2 *scene)
{
	// TODO:
	// block some actions, when there is no scene
	// ...
	// 

}

void QtMainWindow::SceneCommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
	if(scene == GetCurrentScene())
	{
		LoadUndoRedoState(scene);
	}
}

void QtMainWindow::UpdateRulerToolLength(SceneEditor2 *scene, double length, double previewLength)
{
	QString l = QString("Current length: %1").arg(length);
	QString pL = QString("Preview length: %1").arg(previewLength);

	QString msg;
	if (length >= 0.0)
	{
		msg = l;
	}
	if (previewLength >= 0.0)
	{
		msg += ";    " + pL;
	}

	ui->statusBar->showMessage(msg);
}

// ###################################################################################################
// Mainwindow Qt actions
// ###################################################################################################

void QtMainWindow::OnProjectOpen()
{
	QString newPath = ProjectManager::Instance()->ProjectOpenDialog();
	ProjectManager::Instance()->ProjectOpen(newPath);
}

void QtMainWindow::OnProjectClose()
{
	// TODO:
	// Close all scenes
	// ...
	// 

	ProjectManager::Instance()->ProjectClose();
}

void QtMainWindow::OnSceneNew()
{
	int index = ui->sceneTabWidget->OpenTab();
	ui->sceneTabWidget->SetCurrentTab(index);
}

void QtMainWindow::OnSceneOpen()
{
	QString path = QFileDialog::getOpenFileName(this, "Open scene file", ProjectManager::Instance()->CurProjectDataSourcePath(), "DAVA Scene V2 (*.sc2)");
	if (!path.isEmpty())
	{
		int index = ui->sceneTabWidget->OpenTab(DAVA::FilePath(path.toStdString()));
		ui->sceneTabWidget->SetCurrentTab(index);

		AddRecent(path);
	}
}

void QtMainWindow::OnSceneSave()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		DAVA::FilePath scenePath = scene->GetScenePath();
		if(!scene->IsLoaded() || scenePath.IsEmpty())
		{
			SaveSceneAs(scene);
		} 
		else
		{
			if(scene->IsChanged())
			{
				if(!scene->Save(scenePath))
				{
					QMessageBox::warning(this, "Save error", "An error occurred while saving the scene. See log for more info.", QMessageBox::Ok);
				}
			}
		}
	}
}

void QtMainWindow::OnSceneSaveAs()
{
	SaveSceneAs(GetCurrentScene());
}

void QtMainWindow::OnSceneSaveToFolder()
{
	// TODO:
	// ...
	// 

}

void QtMainWindow::ExportMenuTriggered(QAction *exportAsAction)
{
	SceneEditor2* scene = GetCurrentScene();
	if (scene)
	{
		eGPUFamily gpuFamily = (eGPUFamily)exportAsAction->data().toInt();
		if (!scene->Export(gpuFamily))
		{
			QMessageBox::warning(this, "Export error", "An error occurred while exporting the scene. See log for more info.", QMessageBox::Ok);
		}
	}
}

void QtMainWindow::OnRecentTriggered(QAction *recentAction)
{
	if(recentScenes.contains(recentAction))
	{
		QString path = recentAction->data().toString();

		int index = ui->sceneTabWidget->OpenTab(DAVA::FilePath(path.toStdString()));
		ui->sceneTabWidget->SetCurrentTab(index);

		if(-1 != index)
		{
			AddRecent(path);
		}
	}
}

void QtMainWindow::OnUndo()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->Undo();
	}
}

void QtMainWindow::OnRedo()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->Redo();
	}
}

void QtMainWindow::OnReloadTextures()
{
	SetGPUFormat(GetGPUFormat());
}

void QtMainWindow::OnReloadTexturesTriggered(QAction *reloadAction)
{
	DAVA::eGPUFamily gpu = (DAVA::eGPUFamily) reloadAction->data().toInt();
	if(gpu >= DAVA::GPU_UNKNOWN && gpu < DAVA::GPU_FAMILY_COUNT)
	{
		// TODO:
		// show wait message
		// ...

		SetGPUFormat(gpu);
	}
}

void QtMainWindow::OnSelectMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->modifSystem->SetModifMode(ST_MODIF_OFF);
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnMoveMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->modifSystem->SetModifMode(ST_MODIF_MOVE);
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnRotateMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->modifSystem->SetModifMode(ST_MODIF_ROTATE);
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnScaleMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->modifSystem->SetModifMode(ST_MODIF_SCALE);
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnPivotCenterMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->selectionSystem->SetPivotPoint(ST_PIVOT_ENTITY_CENTER);
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnPivotCommonMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->selectionSystem->SetPivotPoint(ST_PIVOT_COMMON_CENTER);
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnManualModifMode()
{
	if(ui->actionManualModifMode->isChecked())
	{
		modificationWidget->SetPivotMode(ModificationWidget::PivotRelative);
	}
	else
	{
		modificationWidget->SetPivotMode(ModificationWidget::PivotAbsolute);
	}
}

void QtMainWindow::OnPlaceOnLandscape()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->modifSystem->PlaceOnLandscape(scene->selectionSystem->GetSelection());
	}
}

void QtMainWindow::OnSnapToLandscape()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		scene->modifSystem->SetLandscapeSnap(ui->actionModifySnapToLandscape->isChecked());
		LoadModificationState(scene);
	}
}

void QtMainWindow::OnMaterialEditor()
{
	if(NULL == materialEditor->GetParent())
	{
		SceneEditor2* sceneEditor = GetCurrentScene();
		if(NULL != sceneEditor)
		{
			materialEditor->SetWorkingScene(sceneEditor, sceneEditor->selectionSystem->GetSelection()->GetEntity(0));
		}

		DAVA::UIScreen *curScreen = DAVA::UIScreenManager::Instance()->GetScreen();
		curScreen->AddControl(materialEditor);
	}
	else
	{
		materialEditor->GetParent()->RemoveControl(materialEditor);
		materialEditor->SetWorkingScene(NULL, NULL);
	}
}

void QtMainWindow::OnTextureBrowser()
{
	SceneEditor2* sceneEditor = GetCurrentScene();
	DAVA::Entity *selectedEntity = NULL;

	if(NULL != sceneEditor)
	{
		selectedEntity = sceneEditor->selectionSystem->GetSelection()->GetEntity(0);
	}

	TextureBrowser::Instance()->sceneActivated(sceneEditor);
	TextureBrowser::Instance()->sceneNodeSelected(sceneEditor, selectedEntity); 
	TextureBrowser::Instance()->show();
}

void QtMainWindow::OnSceneLightMode()
{
	SceneEditor2* scene = GetCurrentScene();
	if(NULL != scene)
	{
		if(ui->actionEnableCameraLight->isChecked())
		{
			scene->editorLightSystem->SetCameraLightEnabled(true);
		}
		else
		{
			scene->editorLightSystem->SetCameraLightEnabled(false);
		}

		LoadEditorLightState(scene);
	}
}

void QtMainWindow::OnCubemapEditor()
{
	CubeMapTextureBrowser dlg(dynamic_cast<QWidget*>(parent()));
	dlg.exec();
}

void QtMainWindow::OnNotPassableTerrain()
{
	SceneEditor2* scene = GetCurrentScene();
	if (!scene)
	{
		return;
	}

	bool enabled = scene->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled();
	if (!enabled)
	{
		if (!scene->landscapeEditorDrawSystem->EnableNotPassableTerrain())
		{
			QMessageBox::critical(0, "Error enabling Not Passable Landscape",
								  "Error enabling Not Passable Landscape.\nMake sure there is landscape in scene and disable other landscape editors.");
		}
	}
	else
	{
		scene->landscapeEditorDrawSystem->DisableNotPassableTerrain();
	}

	ui->actionShowNotPassableLandscape->setChecked(scene->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled());
}

void QtMainWindow::OnRulerTool()
{
	SceneEditor2* scene = GetCurrentScene();
	if (!scene)
	{
		return;
	}

	bool enabled = scene->rulerToolSystem->IsLandscapeEditingEnabled();
	if (!enabled)
	{
		if (!scene->rulerToolSystem->EnableLandscapeEditing())
		{
			QMessageBox::critical(0, "Error enabling Ruler Tool",
								  "Error enabling Ruler Tool.\nMake sure there is landscape in scene and disable other landscape editors.");
		}
	}
	else
	{
		scene->rulerToolSystem->DisableLandscapeEdititing();
	}

	ui->actionRulerTool->setChecked(scene->rulerToolSystem->IsLandscapeEditingEnabled());
}

// ###################################################################################################
// Mainwindow load state functions
// ###################################################################################################

void QtMainWindow::LoadModificationState(SceneEditor2 *scene)
{
	if(NULL != scene)
	{
		ui->actionModifySelect->setChecked(false);
		ui->actionModifyMove->setChecked(false);
		ui->actionModifyRotate->setChecked(false);
		ui->actionModifyScale->setChecked(false);

		ST_ModifMode modifMode = scene->modifSystem->GetModifMode();
		modificationWidget->SetModifMode(modifMode);

		switch (modifMode)
		{
		case ST_MODIF_OFF:
			ui->actionModifySelect->setChecked(true);
			break;
		case ST_MODIF_MOVE:
			ui->actionModifyMove->setChecked(true);
			break;
		case ST_MODIF_ROTATE:
			ui->actionModifyRotate->setChecked(true);
			break;
		case ST_MODIF_SCALE:
			ui->actionModifyScale->setChecked(true);
			break;
		default:
			break;
		}


		// pivot point
		if(scene->selectionSystem->GetPivotPoint() == ST_PIVOT_ENTITY_CENTER)
		{
			ui->actionPivotCenter->setChecked(true);
			ui->actionPivotCommon->setChecked(false);
		}
		else
		{
			ui->actionPivotCenter->setChecked(false);
			ui->actionPivotCommon->setChecked(true);
		}

		// landscape snap
		ui->actionModifySnapToLandscape->setChecked(scene->modifSystem->GetLandscapeSnap());
	}
}

void QtMainWindow::LoadUndoRedoState(SceneEditor2 *scene)
{
	if(NULL != scene)
	{
		ui->actionUndo->setEnabled(scene->CanUndo());
		ui->actionRedo->setEnabled(scene->CanRedo());
	}
}

void QtMainWindow::LoadEditorLightState(SceneEditor2 *scene)
{
	if(NULL != scene)
	{
		ui->actionEnableCameraLight->setChecked(scene->editorLightSystem->GetCameraLightEnabled());
	}
}

void QtMainWindow::LoadNotPassableState(SceneEditor2* scene)
{
	if (!scene)
	{
		return;
	}

	ui->actionShowNotPassableLandscape->setChecked(scene->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled());
}

void QtMainWindow::LoadRulerToolState(SceneEditor2* scene)
{
	if (!scene)
	{
		return;
	}

	ui->actionRulerTool->setChecked(scene->rulerToolSystem->IsLandscapeEditingEnabled());
}

void QtMainWindow::LoadGPUFormat()
{
	int curGPU = GetGPUFormat();

	QList<QAction *> allActions = ui->menuTexturesForGPU->actions();
	for(int i = 0; i < allActions.size(); ++i)
	{
		QAction *actionN = allActions[i];

		if(!actionN->data().isNull() &&
			actionN->data().toInt() == curGPU)
		{
			actionN->setChecked(true);
			ui->actionReloadTextures->setText(actionN->text());
		}
		else
		{
			actionN->setChecked(false);
		}
	}
}



#if 0
#include "mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "Classes/Qt/Scene/SceneDataManager.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/CommandLine/CommandLineManager.h"
#include "Classes/Qt/TextureBrowser/TextureConvertor.h"
#include "Classes/Qt/DockSceneGraph/PointerHolder.h"
#include "DockConsole/Console.h"
#include "Classes/Qt/Project/ProjectManager.h"
#include "DockLibrary/LibraryModel.h"

#include <QToolBar>
#include <QToolButton>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/EditorConfig.h"
#include "SpritesPacker/SpritePackerHelper.h"
#include "Tools/QResourceEditorProgressDialog/QResourceEditorProgressDialog.h"

#include <QApplication>
#include <QPixmap>

#include "ModificationWidget.h"

#include "Qt/Scene/System/LandscapeEditorDrawSystem.h"


QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, oldDockSceneGraphMinSize(-1, -1)
	, oldDockSceneGraphMaxSize(-1, -1)
	, repackSpritesWaitDialog(NULL)
	, emitRepackAndReloadFinished(false)
{
	// initialize console
	Console::Instance();

	new ProjectManager();
	new SceneDataManager();
	new QtMainWindowHandler(this);

	ui->setupUi(this);
 
    qApp->installEventFilter(this);
	EditorConfig::Instance()->ParseConfig(EditorSettings::Instance()->GetProjectPath() + "EditorConfig.yaml");

	QtMainWindowHandler::Instance()->SetDefaultFocusWidget(ui->sceneTabWidget);
	QtMainWindowHandler::Instance()->SetResentMenu(ui->menuFile);
	QtMainWindowHandler::Instance()->RegisterStatusBar(ui->statusBar);
	QtMainWindowHandler::Instance()->RestoreDefaultFocus();

    RegisterBasePointerTypes();
   
	SetupActions();
    SetupMainMenu();
    SetupToolBars();
    SetupDocks();

	posSaver.Attach(this);
	posSaver.LoadState(this);
	
	ui->dockParticleEditor->installEventFilter(this);
	ChangeParticleDockVisible(false, true); //hide particle editor dock on start up
	ui->dockParticleEditorTimeLine->hide();

	// Open last project
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	OpenLastProject();
}

QtMainWindow::~QtMainWindow()
{
	posSaver.SaveState(this);

	delete ui;
    
	QtMainWindowHandler::Instance()->Release();
	SceneDataManager::Instance()->Release();
	ProjectManager::Instance()->Release();
}

Ui::MainWindow* QtMainWindow::GetUI()
{
	return ui;
}

void QtMainWindow::SetupActions()
{
	QtMainWindowHandler *actionHandler = QtMainWindowHandler::Instance();

	//File
	connect(ui->menuFile, SIGNAL(triggered(QAction *)), actionHandler, SLOT(FileMenuTriggered(QAction *)));
	connect(ui->actionNewScene, SIGNAL(triggered()), actionHandler, SLOT(NewScene()));
	connect(ui->actionOpenScene, SIGNAL(triggered()), actionHandler, SLOT(OpenScene()));
	connect(ui->actionOpenProject, SIGNAL(triggered()), actionHandler, SLOT(OpenProject()));
	connect(ui->actionSaveScene, SIGNAL(triggered()), actionHandler, SLOT(SaveScene()));
	connect(ui->actionSaveToFolder, SIGNAL(triggered()), actionHandler, SLOT(SaveToFolderWithChilds()));
    
    ui->actionExportPVRIOS->setData(GPU_POWERVR_IOS);
    ui->actionExportPVRAndroid->setData(GPU_POWERVR_ANDROID);
    ui->actionExportTegra->setData(GPU_TEGRA);
    ui->actionExportMali->setData(GPU_MALI);
    ui->actionExportAdreno->setData(GPU_ADRENO);
    ui->actionExportPNG->setData(GPU_UNKNOWN);
	connect(ui->menuExport, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ExportMenuTriggered(QAction *)));
    
    
	connect(ui->actionReloadAll, SIGNAL(triggered()), actionHandler, SLOT(RepackAndReloadTextures()));

	//View
	connect(ui->actionRestoreViews, SIGNAL(triggered()), actionHandler, SLOT(RestoreViews()));

	//Tools
	connect(ui->actionMaterialEditor, SIGNAL(triggered()), actionHandler, SLOT(Materials()));
	connect(ui->actionTextureConverter, SIGNAL(triggered()), actionHandler, SLOT(ConvertTextures()));
	connect(ui->actionHeightMapEditor, SIGNAL(triggered()), actionHandler, SLOT(HeightmapEditor()));
	connect(ui->actionTileMapEditor, SIGNAL(triggered()), actionHandler, SLOT(TilemapEditor()));
	connect(ui->actionRulerTool, SIGNAL(triggered()), actionHandler, SLOT(RulerTool()));
	connect(ui->actionShowSettings, SIGNAL(triggered()), actionHandler, SLOT(ShowSettings()));
    connect(ui->actionSquareTextures, SIGNAL(triggered()), actionHandler, SLOT(SquareTextures()));
    connect(ui->actionShowMipmapLevel, SIGNAL(triggered()), actionHandler, SLOT(ReplaceZeroMipmaps()));
    connect(ui->actionCubemapEditor, SIGNAL(triggered()), actionHandler, SLOT(CubemapEditor()));

#if defined (__DAVAENGINE_MACOS__)
    ui->menuTools->removeAction(ui->actionBeast);
#else //#if defined (__DAVAENGINE_MACOS__)
	connect(ui->actionBeast, SIGNAL(triggered()), actionHandler, SLOT(Beast()));
#endif //#if defined (__DAVAENGINE_MACOS__)
    
	//Edit
	connect(ui->actionConvertToShadow, SIGNAL(triggered()), actionHandler, SLOT(ConvertToShadow()));

	connect(ui->actionEnableNotPassable, SIGNAL(triggered()), this, SLOT(EnableNotPassableNew()));
    
//     ui->actionEnableCameraLight->setChecked(EditorSettings::Instance()->GetShowEditorCamerLight());
	ui->actionEnableCameraLight->setChecked(true);
	connect(ui->actionEnableCameraLight, SIGNAL(triggered()), actionHandler, SLOT(CameraLightTrigerred()));

    //Help
    connect(ui->actionHelp, SIGNAL(triggered()), actionHandler, SLOT(OpenHelp()));
}

void QtMainWindow::SetupMainMenu()
{
    QtMainWindowHandler *actionHandler = QtMainWindowHandler::Instance();

    QAction *actionSceneGraph = ui->dockSceneGraph->toggleViewAction();
    QAction *actionProperties = ui->dockProperties->toggleViewAction();
    QAction *actionLibrary = ui->dockLibrary->toggleViewAction();
    QAction *actionToolBar = ui->mainToolBar->toggleViewAction();
    QAction *actionCustomColors = ui->dockCustomColors->toggleViewAction();
	QAction *actionVisibilityCheckTool = ui->dockVisibilityTool->toggleViewAction();
	QAction *actionHangingObjects = ui->dockHangingObjects->toggleViewAction();
	QAction *actionSetSwitchIndex = ui->dockSetSwitchIndex->toggleViewAction();
	QAction *actionParticleEditor = ui->dockParticleEditor->toggleViewAction();
	QAction *actionParticleEditorTimeLine = ui->dockParticleEditorTimeLine->toggleViewAction();
    QAction *actionSceneInfo = ui->dockSceneInfo->toggleViewAction();
	QAction *actionSceneTree = ui->dockSceneTree->toggleViewAction();
	QAction *actionConsole = ui->dockConsole->toggleViewAction();
	QAction *actionCustomColors2 = ui->dockCustomColorsEditor->toggleViewAction();
	QAction *actionVisibilityTool2 = ui->dockVisibilityToolEditor->toggleViewAction();
	QAction *actionHeightmapEditor2 = ui->dockHeightmapEditor->toggleViewAction();
	QAction *actionTilemaskEditor2 = ui->dockTilemaskEditor->toggleViewAction();

    ui->menuView->addAction(actionSceneInfo);
    ui->menuView->addAction(actionToolBar);
    ui->menuView->addAction(actionLibrary);
    ui->menuView->addAction(actionProperties);
    ui->menuView->addAction(actionSceneGraph);
    ui->menuView->addAction(actionCustomColors);
	ui->menuView->addAction(actionVisibilityCheckTool);
	ui->menuView->addAction(actionParticleEditor);
	ui->menuView->addAction(actionParticleEditorTimeLine);
	ui->menuView->addAction(actionHangingObjects);
	ui->menuView->addAction(actionSetSwitchIndex);
	ui->menuView->addAction(actionSceneTree);
	ui->menuView->addAction(actionConsole);
	ui->menuView->addAction(actionCustomColors2);
	ui->menuView->addAction(actionVisibilityTool2);
	ui->menuView->addAction(actionHeightmapEditor2);
	ui->menuView->addAction(actionTilemaskEditor2);

    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionToolBar);
    ui->menuView->insertSeparator(actionProperties);


    actionHandler->RegisterDockActions(ResourceEditor::HIDABLEWIDGET_COUNT,
                                       actionSceneGraph,
                                       actionProperties, actionLibrary, actionToolBar,
									   actionCustomColors, actionVisibilityCheckTool, 
									   actionParticleEditor, actionHangingObjects, actionSetSwitchIndex, actionSceneInfo,
									   actionCustomColors2, actionVisibilityTool2, actionHeightmapEditor2,
									   actionTilemaskEditor2);


    ui->dockProperties->hide();
    
    
    //CreateNode
    actionHandler->RegisterNodeActions(ResourceEditor::NODE_COUNT,
                                       ui->actionLandscape,
                                       ui->actionLight,
                                       ui->actionServiceNode,
                                       ui->actionCamera,
                                       ui->actionImposter,
                                       ui->actionParticleEmitter,
                                       ui->actionUserNode,
									   ui->actionSwitchNode,
									   ui->actionParticleEffectNode,
									   ui->actionSkyboxNode
                                       );
    connect(ui->menuCreateNode, SIGNAL(triggered(QAction *)), actionHandler, SLOT(CreateNodeTriggered(QAction *)));

    
    //TODO: need enable flag in future
    ui->actionHeightMapEditor->setCheckable(false);
    ui->actionTileMapEditor->setCheckable(false);
    //ENDOFTODO
    
    
    //Viewport
    connect(ui->menuViewPort, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ViewportTriggered(QAction *)));
    actionHandler->RegisterViewportActions(ResourceEditor::VIEWPORT_COUNT,
                                           ui->actionIPhone,
                                           ui->actionRetina,
                                           ui->actionIPad,
                                           ui->actionDefault
                                       );

	//Edit Options
	connect(ui->actionUndo, SIGNAL(triggered()), actionHandler, SLOT(UndoAction()));
	connect(ui->actionRedo, SIGNAL(triggered()), actionHandler, SLOT(RedoAction()));
	actionHandler->RegisterEditActions(ResourceEditor::EDIT_COUNT, ui->actionUndo, ui->actionRedo);

    //View Options
    connect(ui->actionShowNotPassableLandscape, SIGNAL(triggered()), actionHandler, SLOT(ToggleNotPassableTerrain()));

    ui->actionReloadPoverVRIOS->setData(GPU_POWERVR_IOS);
    ui->actionReloadPoverVRAndroid->setData(GPU_POWERVR_ANDROID);
    ui->actionReloadTegra->setData(GPU_TEGRA);
    ui->actionReloadMali->setData(GPU_MALI);
    ui->actionReloadAdreno->setData(GPU_ADRENO);
    ui->actionReloadPNG->setData(GPU_UNKNOWN);
	connect(ui->menuTexturesForGPU, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ReloadMenuTriggered(QAction *)));
    actionHandler->RegisterTextureGPUActions(GPU_FAMILY_COUNT + 1, ui->actionReloadPoverVRIOS, ui->actionReloadPoverVRAndroid,
                                             ui->actionReloadTegra, ui->actionReloadMali, ui->actionReloadAdreno, ui->actionReloadPNG);

	//Modifications Options
	connect(ui->actionModifySelect, SIGNAL(triggered()), actionHandler, SLOT(ModificationSelect()));
	connect(ui->actionModifyMove, SIGNAL(triggered()), actionHandler, SLOT(ModificationMove()));
	connect(ui->actionModifyRotate, SIGNAL(triggered()), actionHandler, SLOT(ModificationRotate()));
	connect(ui->actionModifyScale, SIGNAL(triggered()), actionHandler, SLOT(ModificationScale()));
	connect(ui->actionModifyPlaceOnLandscape, SIGNAL(triggered()), actionHandler, SLOT(ModificationPlaceOnLand()));
	connect(ui->actionModifySnapToLandscape, SIGNAL(triggered()), actionHandler, SLOT(ModificationSnapToLand()));
	connect(ui->actionModifyReset, SIGNAL(triggered()), actionHandler, SLOT(OnResetModification()));

	actionHandler->RegisterModificationActions(ResourceEditor::MODIFY_COUNT,
											   ui->actionModifySelect,
											   ui->actionModifyMove,
											   ui->actionModifyRotate,
											   ui->actionModifyScale,
											   ui->actionModifyPlaceOnLandscape,
											   ui->actionModifySnapToLandscape);

	actionHandler->MenuViewOptionsWillShow();
}

void QtMainWindow::SetupToolBars()
{
	// add combobox to select current view mode
	// TODO:
	// ... ->
	ui->viewModeToolBar->addAction(ui->actionShowMipmapLevel);
	ui->viewModeToolBar->addSeparator();

	QComboBox *cbox = new QComboBox(NULL);
	cbox->addItem("Normal mode");
	cbox->addItem("Particles mode");
	cbox->addItem("Landscape mode");
	ui->viewModeToolBar->addWidget(cbox);
	// <-

	// add special modifications widget into modificationToolBar
	ModificationWidget* modificationWidget = new ModificationWidget(this);
	ui->modificationToolBar->insertWidget(ui->actionModifyReset, modificationWidget);
	connect(modificationWidget, SIGNAL(ApplyModification(double, double, double)), QtMainWindowHandler::Instance(), SLOT(OnApplyModification(double, double, double)));

	// let "reload" action from mainToolBar be displayed as icon+text
	ShowActionWithText(ui->mainToolBar, ui->actionReloadAll, true);

	// align part of actions from SceneGraph toolbar to the right
	QWidget* spacer = new QWidget(); 
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->scenegraphToolBar->insertWidget(ui->actionAddNewEntity, spacer);
	ui->scenegraphToolBar->insertSeparator(ui->actionAddNewEntity);

	// let "add new entity" action from sceneGraphToolBar be displayed as icon+text
	ShowActionWithText(ui->scenegraphToolBar, ui->actionAddNewEntity, true);

	// align part of actions from Properties toolbar to the right
	spacer = new QWidget(); 
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui->propertiesToolBar->insertWidget(ui->actionAddNewComponent, spacer);
	ui->propertiesToolBar->insertSeparator(ui->actionAddNewComponent);

	// let "add new component" action from propertiesToolBar be displayed as icon+text
	ShowActionWithText(ui->propertiesToolBar, ui->actionAddNewComponent, true);

	// TODO:
	// remove this ->
	// (is done before merging development to to_master)

	cbox->setEnabled(false);
	ui->actionAddNewEntity->setEnabled(false);
	ui->actionRemoveEntity->setEnabled(false);
	ui->actionAddNewComponent->setEnabled(false);
	ui->actionRemoveComponent->setEnabled(false);

	QAction *undoSceneEditor2 = new QAction("Undo2", this);
	QAction *redoSceneEditor2 = new QAction("Redo2", this);

	QObject::connect(undoSceneEditor2, SIGNAL(triggered()), this, SLOT(Undo2()));
	QObject::connect(redoSceneEditor2, SIGNAL(triggered()), this, SLOT(Redo2()));
	
	ui->viewModeToolBar->addAction(undoSceneEditor2);
	ui->viewModeToolBar->addAction(redoSceneEditor2);

	ui->viewModeToolBar->setParent(NULL);

	// <-
}

void QtMainWindow::OpenLastProject()
{
    if(CommandLineManager::Instance() && !CommandLineManager::Instance()->IsCommandLineModeEnabled())
    {
        DAVA::FilePath projectPath = EditorSettings::Instance()->GetProjectPath();

        if(projectPath.IsEmpty())
        {
			projectPath = FilePath(ProjectManager::Instance()->ProjectOpenDialog().toStdString());
        }

        if(projectPath.IsEmpty())
		{
			QtLayer::Instance()->Quit();
		}
		else
		{
			ProjectManager::Instance()->ProjectOpen(QString(projectPath.GetAbsolutePathname().c_str()));
		}
    }
}

void QtMainWindow::SetupDocks()
{
    connect(ui->actionRefreshSceneGraph, SIGNAL(triggered()), QtMainWindowHandler::Instance(), SLOT(RefreshSceneGraph()));
	
	// Yuri Coder. Automatic show/hide of the Particles Timeline
	// is disabled due to DF-1421.
	//connect(ui->dockParticleEditor->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockVisible(bool)));

	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockTimeLineVisible(bool)));
	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditor->widget(), SLOT(OnUpdate()));
	connect(ui->dockParticleEditor->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditorTimeLine->widget(), SLOT(OnUpdate()));
	
	connect(ui->cbShowDAEFiles, SIGNAL(stateChanged(int)), this, SLOT(LibraryFileTypesChanged()));
	connect(ui->cbShowSC2Files, SIGNAL(stateChanged(int)), this, SLOT(LibraryFileTypesChanged()));
	
	connect(this, SIGNAL(LibraryFileTypesChanged(bool, bool)), ui->libraryView, SLOT(LibraryFileTypesChanged(bool, bool)));
}

void QtMainWindow::ChangeParticleDockVisible(bool visible, bool forceUpdate)
{
	if (!forceUpdate && (ui->dockParticleEditor->isVisible() == visible))
		return;

	// ui magic :)
	bool isNeedEmitSignal = false;
	if (oldDockSceneGraphMaxSize.width() < 0)
	{
		oldDockSceneGraphMinSize = ui->dockSceneGraph->minimumSize();
		oldDockSceneGraphMaxSize = ui->dockSceneGraph->maximumSize();
		isNeedEmitSignal = true;
	}
	
	int minWidthParticleDock = ui->dockParticleEditor->minimumWidth();
	int maxWidthParticleDock = ui->dockParticleEditor->maximumWidth();
	
	ui->dockSceneGraph->setFixedWidth(ui->dockSceneGraph->width());
	ui->dockParticleEditor->setFixedWidth(ui->dockParticleEditor->width());
	
	ui->dockParticleEditor->setVisible(visible);
	
	ui->dockParticleEditor->setMinimumWidth(minWidthParticleDock);
	ui->dockParticleEditor->setMaximumWidth(maxWidthParticleDock);
	
	ui->dockSceneGraph->setFixedWidth(ui->dockSceneGraph->width());
	ui->dockParticleEditor->setVisible(visible);
	
	if (isNeedEmitSignal)
		QTimer::singleShot(1, this, SLOT(returnToOldMaxMinSizesForDockSceneGraph()));
}

void QtMainWindow::returnToOldMaxMinSizesForDockSceneGraph()
{
	ui->dockSceneGraph->setMinimumSize(oldDockSceneGraphMinSize);
	ui->dockSceneGraph->setMaximumSize(oldDockSceneGraphMaxSize);
	
	oldDockSceneGraphMinSize = QSize(-1, -1);
	oldDockSceneGraphMaxSize = QSize(-1, -1);
}

void QtMainWindow::ChangeParticleDockTimeLineVisible(bool visible)
{
	// Yuri Coder. Automatic show/hide of the Particles Timeline
	// is disabled due to DF-1234.
	// ui->dockParticleEditorTimeLine->setVisible(visible);
}

void QtMainWindow::ShowActionWithText(QToolBar *toolbar, QAction *action, bool showText)
{
	if(NULL != toolbar && NULL != action)
	{
		QToolButton *toolBnt = dynamic_cast<QToolButton *>(toolbar->widgetForAction(action));
		if(NULL != toolBnt)
		{
			if(showText)
			{
				toolBnt->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
			}
			else
			{
				toolBnt->setToolButtonStyle(Qt::ToolButtonIconOnly);
			}
		}
	}
}

bool QtMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(qApp == obj && ProjectManager::Instance()->IsOpened())
    {
        if(QEvent::ApplicationActivate == event->type())
        {
//            Logger::Debug("QEvent::ApplicationActivate");
            
            if(QtLayer::Instance())
            {
                QtLayer::Instance()->OnResume();
                Core::Instance()->GetApplicationCore()->OnResume();
            }

			// RepackAndReloadScene();			
        }
        else if(QEvent::ApplicationDeactivate == event->type())
        {
//            Logger::Debug("QEvent::ApplicationDeactivate");
            if(QtLayer::Instance())
            {
                QtLayer::Instance()->OnSuspend();
                Core::Instance()->GetApplicationCore()->OnSuspend();
            }
        }
    }
	else if (obj == ui->dockParticleEditor)
	{
		if (QEvent::Close == event->type())
		{
			event->ignore();
			ChangeParticleDockVisible(false);
			return true;
		}
	}
    
    return QMainWindow::eventFilter(obj, event);
}

void QtMainWindow::ProjectOpened(const QString &path)
{
	DAVA::String frameworkTitle = DAVA::Core::Instance()->GetOptions()->GetString("title");
	QString strVer = QString(frameworkTitle.c_str());

	if(!strVer.isEmpty())
	{
		strVer += " | ";
	}

	this->setWindowTitle(strVer + QString("Project - ") + path);
	UpdateLibraryFileTypes();
	UpdateParticleSprites();
}


void QtMainWindow::UpdateParticleSprites()
{
	if(repackSpritesWaitDialog != NULL)
	{
		return;
	}

	repackSpritesWaitDialog = new QResourceEditorProgressDialog(this, 0, true);

	SpritePackerHelper::Instance()->UpdateParticleSpritesAsync();
	
	QObject::connect(SpritePackerHelper::Instance(), SIGNAL(readyAll()), repackSpritesWaitDialog, SLOT(close()));
	QObject::connect(repackSpritesWaitDialog, SIGNAL(destroyed(QObject *)), this, SLOT(RepackSpritesWaitDone(QObject *)));

	repackSpritesWaitDialog->setModal(true);
	repackSpritesWaitDialog->setCancelButton(NULL);
	repackSpritesWaitDialog->setAttribute(Qt::WA_DeleteOnClose);
	repackSpritesWaitDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint);
	repackSpritesWaitDialog->setLabelText("Repack sprites...");
	repackSpritesWaitDialog->setRange(0, 100);
	repackSpritesWaitDialog->setValue(0);
	repackSpritesWaitDialog->show();
}

void QtMainWindow::RepackAndReloadScene()
{
	emitRepackAndReloadFinished = true;
	UpdateParticleSprites();
}

void QtMainWindow::RepackSpritesWaitDone(QObject *destroyed)
{
	if(emitRepackAndReloadFinished)
	{
		emit RepackAndReloadFinished();
	}

	emitRepackAndReloadFinished = false;
	repackSpritesWaitDialog = NULL;
}

void QtMainWindow::LibraryFileTypesChanged()
{
	UpdateLibraryFileTypes();
}

void QtMainWindow::UpdateLibraryFileTypes()
{
	UpdateLibraryFileTypes(ui->cbShowDAEFiles->isChecked(), ui->cbShowSC2Files->isChecked());
}

void QtMainWindow::UpdateLibraryFileTypes(bool showDAEFiles, bool showSC2Files)
{
	emit LibraryFileTypesChanged(showDAEFiles, showSC2Files);
}

void QtMainWindow::EnableNotPassableNew()
{
	SceneEditor2* sep = GetCurrentScene();
	if (!sep)
	{
		return;
	}
	
	if (sep->landscapeEditorDrawSystem->IsNotPassableTerrainEnabled())
	{
		sep->landscapeEditorDrawSystem->DisableNotPassableTerrain();
	}
	else
	{
		sep->landscapeEditorDrawSystem->EnableNotPassableTerrain();
	}
}

SceneEditor2* QtMainWindow::GetCurrentScene()
{
	return ui->sceneTabWidget->GetCurrentScene();
}

void QtMainWindow::Undo2()
{
	SceneEditor2* sceneEditor = ui->sceneTabWidget->GetCurrentScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Undo();
	}
}

void QtMainWindow::Redo2()
{
	SceneEditor2* sceneEditor = ui->sceneTabWidget->GetCurrentScene();
	if(NULL != sceneEditor)
	{
		sceneEditor->Redo();
	}
}

#endif
