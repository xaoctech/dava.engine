#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/QtMainWindowHandler.h"
#include "Classes/Qt/GUIState.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/Qt/SceneDataManager.h"

#include "Classes/Qt/PointerHolder.h"

#include <QToolBar>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/EditorConfig.h"


#include <QApplication>

QtMainWindow::QtMainWindow(QWidget *parent)
    :   QMainWindow(parent)
    ,   ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->centralWidget->setFocus();
 
    qApp->installEventFilter(this);
    
    new QtMainWindowHandler(this);
    QtMainWindowHandler::Instance()->SetDefaultFocusWidget(ui->centralWidget);

    SceneDataManager::Instance()->SetSceneGraphView(ui->sceneGraphTree);
    SceneDataManager::Instance()->SetLibraryView(ui->libraryView);

    
    RegisterBasePointerTypes();
    
    if(DAVA::Core::Instance())
    {
        DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
        if(options)
        {
            QString titleStr(options->GetString("title", "Project Title").c_str());
            this->setWindowTitle(titleStr);
        }
    }
    
    new GUIState();

    SetupMainMenu();
    
    SetupToolBar();
    
    SetupDockWidgets();

    SetupProjectPath();
	EditorConfig::Instance()->ParseConfig(EditorSettings::Instance()->GetProjectPath() + "EditorConfig.yaml");
    
    QtMainWindowHandler::Instance()->RegisterStatusBar(ui->statusBar);
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();
}

QtMainWindow::~QtMainWindow()
{
	QtMainWindowHandler::Instance()->Release();
    
    GUIState::Instance()->Release();
    
    delete ui;
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
    ui->menuView->insertAction(ui->actionRestoreViews, actionToolBar);
    ui->menuView->insertAction(actionToolBar, actionLibrary);
    ui->menuView->insertAction(actionLibrary, actionProperties);
	ui->menuView->insertAction(actionProperties, actionReferences);
    ui->menuView->insertAction(actionReferences, actionEntities);
    ui->menuView->insertAction(actionEntities, actionDataGraph);
    ui->menuView->insertAction(actionDataGraph, actionSceneGraph);
    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionToolBar);
    ui->menuView->insertSeparator(actionProperties);
    actionHandler->RegisterDockActions(ResourceEditor::HIDABLEWIDGET_COUNT,
                                       actionSceneGraph, actionDataGraph, actionEntities,
                                       actionProperties, actionLibrary, actionToolBar, actionReferences);


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
									   ui->actionSwitchNode
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
    connect(ui->actionShowNotPassableLandscape, SIGNAL(triggered()), actionHandler, SLOT(ToggleNotPassableTerrain()));
    
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

void QtMainWindow::SetupProjectPath()
{
    DAVA::String projectPath = EditorSettings::Instance()->GetProjectPath();
    while(0 == projectPath.length())
    {
        QtMainWindowHandler::Instance()->OpenProject();
        projectPath = EditorSettings::Instance()->GetProjectPath();
    }
}

void QtMainWindow::SetupDockWidgets()
{
    ui->sceneGraphTree->setDragDropMode(QAbstractItemView::InternalMove);
    ui->sceneGraphTree->setDragEnabled(true);
    ui->sceneGraphTree->setAcceptDrops(true);
    ui->sceneGraphTree->setDropIndicatorShown(true);
    
    connect(ui->btnRefresh, SIGNAL(clicked()), QtMainWindowHandler::Instance(), SLOT(RefreshSceneGraph()));
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
        }
        else if(QEvent::ApplicationDeactivate == event->type())
        {
            Logger::Debug("QEvent::ApplicationDeactivate");
            if(QtLayer::Instance())
            {
                QtLayer::Instance()->OnSuspend();
                Core::Instance()->GetApplicationCore()->OnResume();
            }
        }
    }
    
    return QMainWindow::eventFilter(obj, event);
}


