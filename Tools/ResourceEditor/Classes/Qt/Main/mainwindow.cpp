#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "Classes/Qt/Main/GUIState.h"
#include "Classes/Qt/Scene/SceneDataManager.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/SceneEditor/CommandLineTool.h"
#include "Classes/Qt/TextureBrowser/TextureConvertor.h"
#include "Classes/Qt/Main/PointerHolder.h"
#include "Classes/Qt/Project/ProjectManager.h"
#include "DockLibrary/LibraryModel.h"

#include <QToolBar>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/EditorConfig.h"
#include "../SceneEditor/CommandLineTool.h"

#include <QApplication>
#include <QPixmap>


QtMainWindow::QtMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
	, convertWaitDialog(NULL)
	, oldDockSceneGraphMinSize(-1, -1)
	, oldDockSceneGraphMaxSize(-1, -1)
{
	new ProjectManager();
	new SceneDataManager();
	new QtMainWindowHandler(this);

	ui->setupUi(this);
	ui->davaGLWidget->setFocus();
 
    qApp->installEventFilter(this);

	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));

	QtMainWindowHandler::Instance()->SetDefaultFocusWidget(ui->davaGLWidget);

    RegisterBasePointerTypes();
   
    new GUIState();

	EditorConfig::Instance()->ParseConfig(EditorSettings::Instance()->GetProjectPath() + "EditorConfig.yaml");
    
    SetupMainMenu();
    SetupToolBar();
    SetupDockWidgets();

    QtMainWindowHandler::Instance()->RegisterStatusBar(ui->statusBar);
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();

	OpenLastProject();

	posSaver.Attach(this);
	posSaver.LoadState(this);
	
	ui->dockParticleEditor->installEventFilter(this);
	ChangeParticleDockVisible(false); //hide particle editor dock on start up
	ui->dockParticleEditorTimeLine->hide();
}

QtMainWindow::~QtMainWindow()
{
	posSaver.SaveState(this);

	GUIState::Instance()->Release();
	delete ui;
    
	QtMainWindowHandler::Instance()->Release();
	SceneDataManager::Instance()->Release();
	ProjectManager::Instance()->Release();

    //SafeDelete(libraryModel);
}

void QtMainWindow::SetupMainMenu()
{
    QtMainWindowHandler *actionHandler = QtMainWindowHandler::Instance();
    //File
    connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->menuFile, SIGNAL(triggered(QAction *)), actionHandler, SLOT(FileMenuTriggered(QAction *)));
    connect(ui->actionNewScene, SIGNAL(triggered()), actionHandler, SLOT(NewScene()));
    connect(ui->actionOpenScene, SIGNAL(triggered()), actionHandler, SLOT(OpenScene()));
    connect(ui->actionOpenProject, SIGNAL(triggered()), actionHandler, SLOT(OpenProject()));
    connect(ui->actionSaveScene, SIGNAL(triggered()), actionHandler, SLOT(SaveScene()));
    connect(ui->actionSaveToFolder, SIGNAL(triggered()), actionHandler, SLOT(SaveToFolderWithChilds()));
    connect(ui->actionPNG, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPNG()));
    connect(ui->actionPVR, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPVR()));
    connect(ui->actionDXT, SIGNAL(triggered()), actionHandler, SLOT(ExportAsDXT()));

    
    //View
    connect(ui->actionSceneInfo, SIGNAL(triggered()), actionHandler, SLOT(ToggleSceneInfo()));
    connect(ui->actionRestoreViews, SIGNAL(triggered()), actionHandler, SLOT(RestoreViews()));
    QAction *actionSceneGraph = ui->dockSceneGraph->toggleViewAction();
    QAction *actionDataGraph = ui->dockDataGraph->toggleViewAction();
    QAction *actionEntities = ui->dockEntities->toggleViewAction();
    QAction *actionProperties = ui->dockProperties->toggleViewAction();
    QAction *actionLibrary = ui->dockLibrary->toggleViewAction();
	QAction *actionReferences = ui->dockReferences->toggleViewAction();
    QAction *actionToolBar = ui->mainToolBar->toggleViewAction();
    QAction *actionCustomColors = ui->dockCustomColors->toggleViewAction();
	QAction *actionVisibilityCheckTool = ui->dockVisibilityTool->toggleViewAction();
	QAction *actionParticleEditor = ui->dockParticleEditor->toggleViewAction();
	QAction *actionParticleEditorTimeLine = ui->dockParticleEditorTimeLine->toggleViewAction();
    ui->menuView->insertAction(ui->actionRestoreViews, actionToolBar);
    ui->menuView->insertAction(actionToolBar, actionLibrary);
    ui->menuView->insertAction(actionLibrary, actionProperties);
	ui->menuView->insertAction(actionProperties, actionReferences);
    ui->menuView->insertAction(actionReferences, actionEntities);
    ui->menuView->insertAction(actionEntities, actionDataGraph);
    ui->menuView->insertAction(actionDataGraph, actionSceneGraph);
    ui->menuView->insertAction(actionSceneGraph, actionCustomColors);
	ui->menuView->insertAction(actionCustomColors, actionVisibilityCheckTool);
	ui->menuView->insertAction(actionVisibilityCheckTool, actionParticleEditor);
	ui->menuView->insertAction(actionParticleEditor, actionParticleEditorTimeLine);
    
    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionToolBar);
    ui->menuView->insertSeparator(actionProperties);
    actionHandler->RegisterDockActions(ResourceEditor::HIDABLEWIDGET_COUNT,
                                       actionSceneGraph, actionDataGraph, actionEntities,
                                       actionProperties, actionLibrary, actionToolBar, actionReferences, actionCustomColors, actionVisibilityCheckTool, actionParticleEditor);


    ui->dockDataGraph->hide();
    ui->dockEntities->hide();
    ui->dockProperties->hide();
	//ui->dockReferences->hide();
    
    
    //CreateNode
    actionHandler->RegisterNodeActions(ResourceEditor::NODE_COUNT,
                                       ui->actionLandscape,
                                       ui->actionLight,
                                       ui->actionServiceNode,
                                       ui->actionBox,
                                       ui->actionSphere,
                                       ui->actionCamera,
                                       ui->actionImposter,
                                       ui->actionParticleEmitter,
                                       ui->actionUserNode,
									   ui->actionSwitchNode,
									   ui->actionParticleEffectNode
                                       );
    connect(ui->menuCreateNode, SIGNAL(triggered(QAction *)), actionHandler, SLOT(CreateNodeTriggered(QAction *)));

    //Tools
    connect(ui->menuTools, SIGNAL(aboutToShow()), actionHandler, SLOT(MenuToolsWillShow()));
    connect(ui->actionMaterialEditor, SIGNAL(triggered()), actionHandler, SLOT(Materials()));
    connect(ui->actionTextureConverter, SIGNAL(triggered()), actionHandler, SLOT(ConvertTextures()));
    connect(ui->actionHeightMapEditor, SIGNAL(triggered()), actionHandler, SLOT(HeightmapEditor()));
    connect(ui->actionTileMapEditor, SIGNAL(triggered()), actionHandler, SLOT(TilemapEditor()));
    connect(ui->actionRulerTool, SIGNAL(triggered()), actionHandler, SLOT(RulerTool()));
    
    connect(ui->actionShowSettings, SIGNAL(triggered()), actionHandler, SLOT(ShowSettings()));
    connect(ui->actionBakeScene, SIGNAL(triggered()), actionHandler, SLOT(BakeScene()));
    connect(ui->actionBeast, SIGNAL(triggered()), actionHandler, SLOT(Beast()));

    
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

    //View Options
    connect(ui->menuViewOptions, SIGNAL(aboutToShow()), actionHandler, SLOT(MenuViewOptionsWillShow()));
    connect(ui->actionShowNotPassableLandscape, SIGNAL(triggered()), actionHandler, SLOT(ToggleNotPassableTerrain()));
    connect(ui->actionReloadAsPNG, SIGNAL(triggered()), actionHandler, SLOT(ReloadAsPNG()));
    connect(ui->actionReloadAsPVR, SIGNAL(triggered()), actionHandler, SLOT(ReloadAsPVR()));
    connect(ui->actionReloadAsDXT, SIGNAL(triggered()), actionHandler, SLOT(ReloadAsDXT()));
    actionHandler->RegisterTextureFormatActions(FILE_FORMAT_COUNT, ui->actionReloadAsPNG, ui->actionReloadAsPVR, ui->actionReloadAsDXT);

	//Reference
	connect(ui->applyReferenceSuffixButton, SIGNAL(clicked()), this, SLOT(ApplyReferenceNodeSuffix()));
 
}

void QtMainWindow::DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename)
{
	QIcon icon;
	icon.addFile(iconFilename, QSize(), QIcon::Normal, QIcon::Off);
	decoratedAction->setIcon(icon);
}


void QtMainWindow::SetupToolBar()
{
 	DecorateWithIcon(ui->actionNewScene, QString::fromUtf8(":/QtIcons/newscene.png"));
 	DecorateWithIcon(ui->actionOpenScene, QString::fromUtf8(":/QtIcons/openscene.png"));
 	DecorateWithIcon(ui->actionOpenProject, QString::fromUtf8(":/QtIcons/openproject.png"));
 	DecorateWithIcon(ui->actionSaveScene, QString::fromUtf8(":/QtIcons/savescene.png"));

 	DecorateWithIcon(ui->actionMaterialEditor, QString::fromUtf8(":/QtIcons/materialeditor.png"));
 	DecorateWithIcon(ui->actionTileMapEditor, QString::fromUtf8(":/QtIcons/tilemapeditor.png"));
 	DecorateWithIcon(ui->actionHeightMapEditor, QString::fromUtf8(":/QtIcons/heightmapeditor.png"));
 	DecorateWithIcon(ui->actionRulerTool, QString::fromUtf8(":/QtIcons/rulertool.png"));
    
 	DecorateWithIcon(ui->actionShowNotPassableLandscape, QString::fromUtf8(":/QtIcons/notpassableterrain.png"));
    
	ui->mainToolBar->addAction(ui->actionNewScene);
    ui->mainToolBar->addAction(ui->actionOpenScene);
    ui->mainToolBar->addAction(ui->actionSaveScene);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionMaterialEditor);

    ui->mainToolBar->addAction(ui->actionHeightMapEditor);
    ui->mainToolBar->addAction(ui->actionTileMapEditor);
    ui->mainToolBar->addAction(ui->actionRulerTool);

    ui->mainToolBar->addSeparator();
    QAction *reloadTexturesAction = ui->mainToolBar->addAction(QString("Reload Textures"));
    DecorateWithIcon(reloadTexturesAction, QString::fromUtf8(":/QtIcons/reloadtextures.png"));
    connect(reloadTexturesAction, SIGNAL(triggered()), QtMainWindowHandler::Instance(), SLOT(ReloadTexturesFromFileSystem()));
    ui->mainToolBar->addSeparator();
    
    ui->mainToolBar->addAction(ui->actionShowNotPassableLandscape);
    ui->mainToolBar->addSeparator();
}

void QtMainWindow::OpenLastProject()
{
    if(!CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter")) && !CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")))
    {
        DAVA::String projectPath = EditorSettings::Instance()->GetProjectPath();

        if(projectPath.empty())
        {
			projectPath = ProjectManager::Instance()->ProjectOpenDialog().toStdString().c_str();
        }

		if(projectPath.empty())
		{
			QtLayer::Instance()->Quit();
		}
		else
		{
			ProjectManager::Instance()->ProjectOpen(QString(projectPath.c_str()));
		}
    }
}

void QtMainWindow::SetupDockWidgets()
{
    ui->sceneGraphTree->setDragDropMode(QAbstractItemView::InternalMove);
    ui->sceneGraphTree->setDragEnabled(true);
    ui->sceneGraphTree->setAcceptDrops(true);
    ui->sceneGraphTree->setDropIndicatorShown(true);

    connect(ui->btnRefresh, SIGNAL(clicked()), QtMainWindowHandler::Instance(), SLOT(RefreshSceneGraph()));
	connect(ui->dockParticleEditor->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockVisible(bool)));
	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ChangeVisible(bool)), this, SLOT(ChangeParticleDockTimeLineVisible(bool)));
	connect(ui->dockParticleEditorTimeLine->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditor->widget(), SLOT(OnUpdate()));
	connect(ui->dockParticleEditor->widget(), SIGNAL(ValueChanged()), ui->dockParticleEditorTimeLine->widget(), SLOT(OnUpdate()));
}

void QtMainWindow::ChangeParticleDockVisible(bool visible)
{
	if (ui->dockParticleEditor->isVisible() == visible)
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
	ui->dockParticleEditorTimeLine->setVisible(visible);
}

void QtMainWindow::MenuFileWillShow()
{
    QtMainWindowHandler::Instance()->SetResentMenu(ui->menuFile);
    QtMainWindowHandler::Instance()->SetResentAncorAction(ui->actionSaveScene);
    QtMainWindowHandler::Instance()->MenuFileWillShow();
}

void QtMainWindow::ApplyReferenceNodeSuffix()
{
	QString qStr = ui->referenceSuffixEdit->text();
	QByteArray array = qStr.toLatin1();
	char * chars = array.data();
	String str(chars);

	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	Scene * scene = screen->FindCurrentBody()->bodyControl->GetScene();
	scene->SetReferenceNodeSuffix(str);
}

bool QtMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(qApp == obj)
    {
        if(QEvent::ApplicationActivate == event->type())
        {
            Logger::Debug("QEvent::ApplicationActivate");
            
            if(QtLayer::Instance())
            {
                QtLayer::Instance()->OnResume();
                Core::Instance()->GetApplicationCore()->OnResume();
            }

			TextureCheckConvetAndWait();
        }
        else if(QEvent::ApplicationDeactivate == event->type())
        {
            Logger::Debug("QEvent::ApplicationDeactivate");
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

	this->setWindowTitle(strVer + path);
}

void QtMainWindow::TextureCheckConvetAndWait(bool forceConvertAll)
{
	if(CommandLineTool::Instance() && !CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter")) && !CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")) && NULL == convertWaitDialog)
	{
		// check if we have textures to convert - 
		// if we have function will return true and conversion will start in new thread
		// signal 'readyAll' will be emited when convention finishes
		if(TextureConvertor::Instance()->checkAndCompressAll(forceConvertAll))
		{
			convertWaitDialog = new QProgressDialog(this);
			QObject::connect(TextureConvertor::Instance(), SIGNAL(readyAll()), convertWaitDialog, SLOT(close()));
			QObject::connect(TextureConvertor::Instance(), SIGNAL(convertStatus(const QString &, int, int)), this, SLOT(ConvertWaitStatus(const QString &, int, int)));
			QObject::connect(convertWaitDialog, SIGNAL(destroyed(QObject *)), this, SLOT(ConvertWaitDone(QObject *)));
			convertWaitDialog->setModal(true);
			convertWaitDialog->setCancelButton(NULL);
			convertWaitDialog->setAttribute(Qt::WA_DeleteOnClose);
			convertWaitDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint);
			convertWaitDialog->show();
		}
	}
}

void QtMainWindow::ConvertWaitDone(QObject *destroyed)
{
	convertWaitDialog = NULL;
}

void QtMainWindow::ConvertWaitStatus(const QString &curPath, int curJob, int jobCount)
{
	if(NULL != convertWaitDialog)
	{
		convertWaitDialog->setRange(0, jobCount);
		convertWaitDialog->setValue(curJob);
		convertWaitDialog->setLabelText(curPath);
	}
}
