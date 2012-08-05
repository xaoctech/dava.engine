#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/GUIActionHandler.h"
#include "Classes/Qt/GUIState.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/Qt/SceneDataManager.h"

QtMainWindow::QtMainWindow(QWidget *parent) 
    :   QMainWindow(parent)
    ,   ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->centralWidget->setFocus();
    
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
    ui->menuView->insertAction(ui->actionRestoreViews, actionLibrary);
    ui->menuView->insertAction(actionLibrary, actionProperties);
    ui->menuView->insertAction(actionProperties, actionEntities);
    ui->menuView->insertAction(actionEntities, actionDataGraph);
    ui->menuView->insertAction(actionDataGraph, actionSceneGraph);
    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionProperties);
    actionHandler->RegisterDockActions(ResourceEditor::DOCK_COUNT,
                                       actionSceneGraph, actionDataGraph, actionEntities,
                                       actionProperties, actionLibrary);
    
    
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

