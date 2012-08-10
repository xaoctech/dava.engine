#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/GUIActionHandler.h"
#include "Classes/Qt/GUIState.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/Qt/SceneDataManager.h"

#include "Classes/Qt/QtUtils.h"

#include <QToolBar>

QtMainWindow::QtMainWindow(QWidget *parent) 
    :   QMainWindow(parent)
    ,   ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->centralWidget->setFocus();
 
    qRegisterMetaTypeStreamOperators<PointerHolder>("PointerHolder");
    qRegisterMetaTypeStreamOperators<QList<PointerHolder> >("QList<PointerHolder>");
    
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
    SetupProjectPath();

    actionHandler = new GUIActionHandler(this);
    SetupMainMenu();
    SetupToolBar();
    
    SetupDockWidgets();
}

QtMainWindow::~QtMainWindow()
{
    DAVA::SafeDelete(actionHandler);
    
    GUIState::Instance()->Release();
    
    delete ui;
}

void QtMainWindow::SetupMainMenu()
{
    //File
    connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->actionNewScene, SIGNAL(triggered()), actionHandler, SLOT(NewScene()));
    connect(ui->actionOpenScene, SIGNAL(triggered()), actionHandler, SLOT(OpenScene()));
    connect(ui->actionOpenProject, SIGNAL(triggered()), actionHandler, SLOT(OpenProject()));
    connect(ui->actionSaveScene, SIGNAL(triggered()), actionHandler, SLOT(SaveScene()));
    connect(ui->actionPNG, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPNG()));
    connect(ui->actionPVR, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPVR()));
    connect(ui->actionDXT, SIGNAL(triggered()), actionHandler, SLOT(ExportAsDXT()));

    //Resent files
    connect(ui->menuResentScenes, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ResentSceneTriggered(QAction *)));

    //View
    connect(ui->actionRestoreViews, SIGNAL(triggered()), actionHandler, SLOT(RestoreViews()));
    QAction *actionSceneGraph = ui->dockSceneGraph->toggleViewAction();
    QAction *actionDataGraph = ui->dockDataGraph->toggleViewAction();
    QAction *actionEntities = ui->dockEntities->toggleViewAction();
    QAction *actionProperties = ui->dockProperties->toggleViewAction();
    QAction *actionLibrary = ui->dockLibrary->toggleViewAction();
    QAction *actionToolBar = ui->mainToolBar->toggleViewAction();
    ui->menuView->insertAction(ui->actionRestoreViews, actionToolBar);
    ui->menuView->insertAction(actionToolBar, actionLibrary);
    ui->menuView->insertAction(actionLibrary, actionProperties);
    ui->menuView->insertAction(actionProperties, actionEntities);
    ui->menuView->insertAction(actionEntities, actionDataGraph);
    ui->menuView->insertAction(actionDataGraph, actionSceneGraph);
    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionToolBar);
    ui->menuView->insertSeparator(actionProperties);
    actionHandler->RegisterDockActions(ResourceEditor::HIDABLEWIDGET_COUNT,
                                       actionSceneGraph, actionDataGraph, actionEntities,
                                       actionProperties, actionLibrary, actionToolBar);


    ui->dockDataGraph->hide();
    ui->dockEntities->hide();
    ui->dockProperties->hide();
    
    
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
                                       ui->actionUserNode
                                       );
    connect(ui->menuCreateNode, SIGNAL(triggered(QAction *)), actionHandler, SLOT(CreateNodeTriggered(QAction *)));

    //Tools
    connect(ui->menuTools, SIGNAL(aboutToShow()), actionHandler, SLOT(MenuToolsWillShow()));
    connect(ui->actionMaterialEditor, SIGNAL(triggered()), actionHandler, SLOT(Materials()));
    connect(ui->actionTextureConverter, SIGNAL(triggered()), actionHandler, SLOT(ConvertTextures()));
    connect(ui->actionHeightMapEditor, SIGNAL(triggered()), actionHandler, SLOT(HeightmapEditor()));
    connect(ui->actionTileMapEditor, SIGNAL(triggered()), actionHandler, SLOT(TilemapEditor()));
    
    //Viewport
    connect(ui->menuViewPort, SIGNAL(triggered(QAction *)), actionHandler, SLOT(ViewportTriggered(QAction *)));
    actionHandler->RegisterViewportActions(ResourceEditor::VIEWPORT_COUNT,
                                           ui->actionIPhone,
                                           ui->actionRetina,
                                           ui->actionIPad,
                                           ui->actionDefault
                                       );
    
    
}

void QtMainWindow::DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename)
{
	QIcon icon;
	icon.addFile(iconFilename, QSize(), QIcon::Normal, QIcon::Off);
	decoratedAction->setIcon(icon);
}


void QtMainWindow::SetupToolBar()
{
// 	DecorateWithIcon(ui->actionNewScene, QString::fromUtf8(":/Data/QtIcons/savescene.png"));
// 	DecorateWithIcon(ui->actionOpenScene, QString::fromUtf8(":/Data/QtIcons/savescene.png"));
// 	DecorateWithIcon(ui->actionOpenProject, QString::fromUtf8(":/Data/QtIcons/savescene.png"));
// 	DecorateWithIcon(ui->actionSaveScene, QString::fromUtf8(":/Data/QtIcons/savescene.png"));

    
	ui->mainToolBar->addAction(ui->actionNewScene);
    ui->mainToolBar->addAction(ui->actionOpenScene);
    ui->mainToolBar->addAction(ui->actionOpenProject);
    ui->mainToolBar->addAction(ui->actionSaveScene);
    ui->mainToolBar->addSeparator();
    
    ui->mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}

void QtMainWindow::SetupProjectPath()
{
    DAVA::String projectPath = EditorSettings::Instance()->GetProjetcPath();
    while(0 == projectPath.length())
    {
        actionHandler->OpenProject();
        projectPath = EditorSettings::Instance()->GetProjetcPath();
    }
}

void QtMainWindow::SetupDockWidgets()
{
    SceneDataManager::Instance()->SetSceneGraphView(ui->sceneGraphTree);
    ui->sceneGraphTree->setDragDropMode(QAbstractItemView::InternalMove);
    ui->sceneGraphTree->setDragEnabled(true);
    ui->sceneGraphTree->setAcceptDrops(true);
    ui->sceneGraphTree->setDropIndicatorShown(true);

    actionHandler->SetLibraryView(ui->libraryView);
    SceneDataManager::Instance()->SetLibraryView(ui->libraryView);
    connect(ui->libraryView, SIGNAL(customContextMenuRequested(const QPoint &)), actionHandler, SLOT(LibraryContextMenuRequested(const QPoint &)));
    
    connect(ui->btnRemoveRootNodes, SIGNAL(clicked()), actionHandler, SLOT(RemoveRootNodes()));
    connect(ui->btnRefresh, SIGNAL(clicked()), actionHandler, SLOT(RefreshSceneGraph()));
    connect(ui->btnLockAtObject, SIGNAL(clicked()), actionHandler, SLOT(LockAtObject()));
    connect(ui->btnRemoveObject, SIGNAL(clicked()), actionHandler, SLOT(RemoveObject()));
    connect(ui->btnDebugFlags, SIGNAL(clicked()), actionHandler, SLOT(DebugFlags()));
    connect(ui->btnBakeMatrices, SIGNAL(clicked()), actionHandler, SLOT(BakeMatrixes()));
    connect(ui->btnBuildQuadTree, SIGNAL(clicked()), actionHandler, SLOT(BuildQuadTree()));
}

void QtMainWindow::MenuFileWillShow()
{
    actionHandler->SetResentMenu(ui->menuResentScenes);
    actionHandler->MenuFileWillShow();
}

