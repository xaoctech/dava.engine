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
#include "Scene3D/Components/SkyboxComponent.h"
#include "Scene3D/Systems/SkyboxSystem.h"

#include "../Tools/BaseAddEntityDialog/BaseAddEntityDialog.h"
//#include "../Tools/AddSwitchEntityDialog/AddSwitchEntityDialog.h"
#include "../Tools/SelectPathWidget/SelectPathWidget.h"

#include "../Tools/AddSwitchEntityDialog/AddSwitchEntityDialog.h"
#include "../../Commands2/AddEntityCommand.h"
#include "StringConstants.h"


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

	addSwitchEntityDialog = new AddSwitchEntityDialog( NULL, dynamic_cast<QWidget*>(QObject::parent()));
}

QtMainWindow::~QtMainWindow()
{
	materialEditor->Release();
	TextureBrowser::Instance()->Release();

	posSaver.SaveState(this);

	delete ui;
	ui = NULL;

	ProjectManager::Instance()->Release();
	delete addSwitchEntityDialog;
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
		WaitStart("Reloading textures...", "", 0, allScenesTextures.size());

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
	waitDialog->Show(title, message, false, false);
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

	QObject::connect(ui->menuAdd, SIGNAL(aboutToShow()), this, SLOT(OnAddEntityMenuAboutToShow()));
	QObject::connect(ui->actionSkyboxNode, SIGNAL(triggered()), this, SLOT(OnAddSkyboxNode()));

	QObject::connect(ui->actionLandscape, SIGNAL(triggered()), this, SLOT(OnLandscapeDialog()));
	QObject::connect(ui->actionLight, SIGNAL(triggered()), this, SLOT(OnLightDialog()));
	QObject::connect(ui->actionServiceNode, SIGNAL(triggered()), this, SLOT(OnServiceNodeDialog()));
	QObject::connect(ui->actionCamera, SIGNAL(triggered()), this, SLOT(OnCameraDialog()));
	QObject::connect(ui->actionImposter, SIGNAL(triggered()), this, SLOT(OnImposterDialog()));
	QObject::connect(ui->actionUserNode, SIGNAL(triggered()), this, SLOT(OnUserNodeDialog()));
	QObject::connect(ui->actionSwitchNode, SIGNAL(triggered()), this, SLOT(OnSwitchEntityDialog()));
	QObject::connect(ui->actionParticleEffectNode, SIGNAL(triggered()), this, SLOT(OnParticleEffectDialog()));
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

void QtMainWindow::AddSwitchDialogFinished(int result)
{
	QObject::disconnect(addSwitchEntityDialog, SIGNAL(finished(int)), this, SLOT(AddSwitchDialogFinished(int)));

	SceneEditor2* scene = GetCurrentScene();

	Entity* switchEntity = addSwitchEntityDialog->GetEntity();

	if(result != QDialog::Accepted || NULL == scene)
	{
		addSwitchEntityDialog->CleanupPathWidgets();
		addSwitchEntityDialog->SetEntity(NULL);
		return;
	}

	Vector<Entity*> vector;
	addSwitchEntityDialog->GetPathEntities(vector, scene);	
	addSwitchEntityDialog->CleanupPathWidgets();
	
	Q_FOREACH(Entity* item, vector)
	{
		switchEntity->AddNode(item);
	}
	if(vector.size())
	{
		scene->Exec(new AddEntityCommand(switchEntity, scene));
	}
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

void QtMainWindow::OnAddSkyboxNode()
{
	SceneEditor2* scene = GetCurrentScene();
	if (!scene)
	{
		return;
	}
	
	if(scene->skyboxSystem->IsSkyboxPresent())
	{
		QMessageBox::warning(0, tr("Skybox was not added"), tr("There's a skybox present in the scene already! Please remove it to add another one."));
		return;
	}

	Entity* skyboxNode = new Entity();
	skyboxNode->SetName("Skybox-singleton");
	SkyboxComponent* component = new SkyboxComponent();
	skyboxNode->AddComponent(component);
	
	scene->AddNode(skyboxNode);
	
	scene->selectionSystem->SetSelection(skyboxNode);
}

void QtMainWindow::OnAddEntityMenuAboutToShow()
{
	SceneEditor2* scene = GetCurrentScene();
	if (!scene)
	{
		return;
	}
	
	//disable adding of skybox if it was present
	ui->actionSkyboxNode->setEnabled(!scene->skyboxSystem->IsSkyboxPresent());
}
void QtMainWindow::OnSwitchEntityDialog()
{
	if(addSwitchEntityDialog->GetEntity() != NULL)//dialog is on screen, do nothing
	{
		return;
	}
	
	Entity* entityToAdd = new Entity();
	entityToAdd->SetName(ResourceEditor::SWITCH_NODE_NAME);
	entityToAdd->AddComponent(new SwitchComponent());
	KeyedArchive *customProperties = entityToAdd->GetCustomProperties();
	customProperties->SetBool(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
	addSwitchEntityDialog->SetEntity(entityToAdd);
	
	QObject::connect(addSwitchEntityDialog, SIGNAL(finished(int)), this, SLOT(AddSwitchDialogFinished(int)));
	addSwitchEntityDialog->show();
}


void QtMainWindow::OnLandscapeDialog()
{
	Entity* sceneNode = new Entity();
	sceneNode->AddComponent(new RenderComponent(ScopedPtr<Landscape>(new Landscape())));
	sceneNode->SetName(ResourceEditor::LANDSCAPE_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
}

void QtMainWindow::OnLightDialog()
{
	Entity* sceneNode = new Entity();
	sceneNode->AddComponent(new LightComponent(ScopedPtr<Light>(new Light)));
	sceneNode->SetName(ResourceEditor::LIGHT_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
}

void QtMainWindow::OnServiceNodeDialog()
{	
	Entity* sceneNode = new Entity();
	KeyedArchive *customProperties = sceneNode->GetCustomProperties();
	customProperties->SetBool("editor.isLocked", true);
	sceneNode->SetName(ResourceEditor::SERVICE_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
}

void QtMainWindow::OnCameraDialog()
{
	Entity* sceneNode = new Entity();
	Camera * camera = new Camera();
	camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
	sceneNode->AddComponent(new CameraComponent(camera));
	sceneNode->SetName(ResourceEditor::CAMERA_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
	SafeRelease(camera);
}

void QtMainWindow::OnImposterDialog()
{
	Entity* sceneNode = new ImposterNode();
	sceneNode->SetName(ResourceEditor::IMPOSTER_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
}

void QtMainWindow::OnUserNodeDialog()
{
	Entity* sceneNode = new Entity();
	sceneNode->AddComponent(new UserComponent());
	sceneNode->SetName(ResourceEditor::USER_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
}

void QtMainWindow::OnParticleEffectDialog()
{
	Entity* sceneNode = new Entity();
	sceneNode->AddComponent(new ParticleEffectComponent());
	sceneNode->SetName(ResourceEditor::PARTICLE_EFFECT_NODE_NAME);
	CreateAndDisplayAddEntityDialog(sceneNode);
}


void QtMainWindow::CreateAndDisplayAddEntityDialog(Entity* sceneNode)
{
	SceneEditor2* sceneEditor = GetCurrentScene();
	BaseAddEntityDialog* dlg = new BaseAddEntityDialog(sceneNode, dynamic_cast<QWidget*>(QObject::parent()));
	dlg->exec();
	if(dlg->result() == QDialog::Accepted && sceneEditor)
	{
		sceneEditor->Exec(new AddEntityCommand(sceneNode, sceneEditor));
	}
	else
	{
		SafeRelease(sceneNode);
	}
	delete dlg;
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
