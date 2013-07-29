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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "ModificationWidget.h"

#include "Base/Singleton.h"

#include "Scene/SceneEditor2.h"
#include "Tools/QtPosSaver/QtPosSaver.h"

// TODO: remove old screen -->
#include "Classes/SceneEditor/MaterialEditor.h"
// <---

class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
	Q_OBJECT

public:
	explicit QtMainWindow(QWidget *parent = 0);
	~QtMainWindow();

	Ui::MainWindow* GetUI();
	SceneTabWidget* GetSceneWidget();
	SceneEditor2* GetCurrentScene();

	bool SaveSceneAs(SceneEditor2 *scene);

// qt actions slots
public slots:
	void OnProjectOpen();
	void OnProjectClose();
	void OnSceneNew();
	void OnSceneOpen();
	void OnSceneSave();
	void OnSceneSaveAs();
	void OnSceneSaveToFolder();
	void ExportMenuTriggered(QAction *exportAsAction);

	void OnUndo();
	void OnRedo();

	void OnSelectMode();
	void OnMoveMode();
	void OnRotateMode();
	void OnScaleMode();
	void OnPivotCenterMode();
	void OnPivotCommonMode();
	void OnManualModifMode();
	void OnPlaceOnLandscape();
	void OnSnapToLandscape();

	void OnMaterialEditor();
	void OnTextureBrowser();
	void OnSceneLightMode();

	void OnCubemapEditor();

	void OnNotPassableTerrain();
	void OnRulerTool();

protected:
	virtual bool eventFilter(QObject *object, QEvent *event);

	void SetupMainMenu();
	void SetupToolBars();
	void SetupDocks();
	void SetupActions();

protected slots:
	void ProjectOpened(const QString &path);
	void ProjectClosed();

	void SceneCommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);
	void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);

	void UpdateRulerToolLength(SceneEditor2* scene, double length, double previewLength);

private:
	Ui::MainWindow *ui;
	QtPosSaver posSaver;

	ModificationWidget *modificationWidget;

	// TODO: remove this old screen -->
	MaterialEditor *materialEditor;
	// <--

	void LoadUndoRedoState(SceneEditor2 *scene);
	void LoadModificationState(SceneEditor2 *scene);
	void LoadEditorLightState(SceneEditor2 *scene);
	void LoadNotPassableState(SceneEditor2* scene);
	void LoadRulerToolState(SceneEditor2* scene);
};

#if 0
#include <QMainWindow>
#include <QProgressDialog>
#include "Base/Singleton.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "ui_mainwindow.h"


class LibraryModel;
class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
    Q_OBJECT
    
public:
	explicit QtMainWindow(QWidget *parent = 0);
	~QtMainWindow();

	Ui::MainWindow* GetUI();
    
    virtual bool eventFilter(QObject *, QEvent *);

	SceneEditor2* GetCurrentScene();

private:
	void OpenLastProject();

	void SetupActions();
    void SetupMainMenu();
	void SetupToolBars();
    void SetupDocks();

    void SetupCustomColorsDock();
	void SetupVisibilityToolDock();
    
    void SetCustomColorsDockControlsEnabled(bool enabled);

	void UpdateLibraryFileTypes();
	void UpdateLibraryFileTypes(bool showDAEFiles, bool showSC2Files);

public slots:
	void ShowActionWithText(QToolBar *toolbar, QAction *action, bool showText);

	void ChangeParticleDockVisible(bool visible, bool forceUpdate = false);
	void ChangeParticleDockTimeLineVisible(bool visible);
	void returnToOldMaxMinSizesForDockSceneGraph();

	//return true if conversion has been started
	void UpdateParticleSprites();
	void RepackAndReloadScene();

	void EnableNotPassableNew();

	void Undo2();
	void Redo2();

private slots:
	void ProjectOpened(const QString &path);
	void LibraryFileTypesChanged();
	
	void RepackSpritesWaitDone(QObject *destroyed);

signals:
	// Library File Types.
	void LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files);
	void RepackAndReloadFinished();

private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;

	QProgressDialog *repackSpritesWaitDialog;
    
	QSize oldDockSceneGraphMaxSize;
	QSize oldDockSceneGraphMinSize;

	bool emitRepackAndReloadFinished;
};

#endif

#endif // MAINWINDOW_H
