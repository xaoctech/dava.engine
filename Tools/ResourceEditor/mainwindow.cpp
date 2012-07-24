#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/GUIActionHandler.h"

#include "Classes/SceneEditor/EditorSettings.h"

MainWindow::MainWindow(QWidget *parent) 
    :   QMainWindow(parent)
    ,   ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
  
	ui->davaGlWidget->setFocus();
    
    if(DAVA::Core::Instance())
    {
        DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
        if(options)
        {
            QString titleStr(options->GetString("title", "Project Title").c_str());
            
            this->setWindowTitle(titleStr);
        }
    }

    actionHandler = new GUIActionHandler(this);
    SetupMainMenu();
    
    SetupProjectPath();
}

MainWindow::~MainWindow()
{
    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        DAVA::SafeDelete(resentSceneActions[i]);
    }
    
    for(DAVA::int32 i = 0; i < ResourceEditor::NODE_COUNT; ++i)
    {
        nodeActions[i] = NULL;
    }
    
    for(DAVA::int32 i = 0; i < ResourceEditor::VIEWPORT_COUNT; ++i)
    {
        viewportActions[i] = NULL;
    }
    
    DAVA::SafeDelete(actionHandler);
    delete ui;
}

void MainWindow::SetupMainMenu()
{
    //File
    connect(ui->actionNewScene, SIGNAL(triggered()), actionHandler, SLOT(NewScene()));
    connect(ui->actionOpenScene, SIGNAL(triggered()), actionHandler, SLOT(OpenScene()));
    connect(ui->actionOpenProject, SIGNAL(triggered()), actionHandler, SLOT(OpenProject()));
    //Resent files
    connect(ui->actionSaveScene, SIGNAL(triggered()), actionHandler, SLOT(SaveScene()));
    connect(ui->actionPNG, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPNG()));
    connect(ui->actionPVR, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPVR()));
    connect(ui->actionDXT, SIGNAL(triggered()), actionHandler, SLOT(ExportAsDXT()));

    //CreateNode
    nodeActions[ResourceEditor::NODE_LANDSCAPE] = ui->actionLandscape;
    nodeActions[ResourceEditor::NODE_LIGHT] = ui->actionLight;
    nodeActions[ResourceEditor::NODE_SERVICE_NODE] = ui->actionServiceNode;
    nodeActions[ResourceEditor::NODE_BOX] = ui->actionBox;
    nodeActions[ResourceEditor::NODE_SPHERE] = ui->actionSphere;
    nodeActions[ResourceEditor::NODE_CAMERA] = ui->actionCamera;
    nodeActions[ResourceEditor::NODE_IMPOSTER] = ui->actionImposter;
    nodeActions[ResourceEditor::NODE_PARTICLE_EMITTER] = ui->actionParticleEmitter;
    nodeActions[ResourceEditor::NODE_USER_NODE] = ui->actionUserNode;
    connect(ui->menuCreateNode, SIGNAL(triggered(QAction *)), this, SLOT(CreateNodeTriggered(QAction *)));

    //TODO: need to updated tools menu gui same way as native editor gui for landscape editors
    //Tools
    connect(ui->actionMaterialEditor, SIGNAL(triggered()), actionHandler, SLOT(Materials()));
    connect(ui->actionTextureConverter, SIGNAL(triggered()), actionHandler, SLOT(ConvertTextures()));
    connect(ui->actionHeightMapEditor, SIGNAL(triggered()), actionHandler, SLOT(HeightmapEditor()));
    connect(ui->actionTileMapEditor, SIGNAL(triggered()), actionHandler, SLOT(TilemapEditor()));
    
    //Viewport
    viewportActions[ResourceEditor::VIEWPORT_IPHONE] = ui->actionIPhone;
    viewportActions[ResourceEditor::VIEWPORT_RETINA] = ui->actionRetina;
    viewportActions[ResourceEditor::VIEWPORT_IPAD] = ui->actionIPad;
    viewportActions[ResourceEditor::VIEWPORT_DEFAULT] = ui->actionDefault;
    connect(ui->menuViewPort, SIGNAL(triggered(QAction *)), this, SLOT(ViewportTriggered(QAction *)));
    
    connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->menuResentScenes, SIGNAL(triggered(QAction *)), this, SLOT(ResentSceneTriggered(QAction *)));
    
    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        resentSceneActions[i] = new QAction(this);
        resentSceneActions[i]->setObjectName(QString::fromUtf8(DAVA::Format("resentSceneActions[%d]", i)));
    }
}

void MainWindow::SetupProjectPath()
{
    DAVA::String projectPath = EditorSettings::Instance()->GetProjetcPath();
    while(0 == projectPath.length())
    {
        actionHandler->OpenProject();
        projectPath = EditorSettings::Instance()->GetProjetcPath();
    }
}

void MainWindow::MenuFileWillShow()
{
    DAVA::int32 sceneCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(0 < sceneCount)
    {
        ui->menuResentScenes->setEnabled(true);

        for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
        {
            if(resentSceneActions[i]->parentWidget())
            {
                ui->menuResentScenes->removeAction(resentSceneActions[i]);
            }
        }
        
        for(DAVA::int32 i = 0; i < sceneCount; ++i)
        {
            resentSceneActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            ui->menuResentScenes->addAction(resentSceneActions[i]);
        }
    }
    else 
    {
        ui->menuResentScenes->setEnabled(false);
    }
}


void MainWindow::ResentSceneTriggered(QAction *resentScene)
{
    if(!actionHandler)  return;
    
    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        if(resentScene == resentSceneActions[i])
        {
            actionHandler->OpenResentScene(i);
        }
    }
}

void MainWindow::CreateNodeTriggered(QAction *nodeAction)
{
    if(!actionHandler)  return;
    
    for(DAVA::int32 i = 0; i < ResourceEditor::NODE_COUNT; ++i)
    {
        if(nodeAction == nodeActions[i])
        {
            actionHandler->CreateNode((ResourceEditor::eNodeType)i);
        }
    }
}

void MainWindow::ViewportTriggered(QAction *viewportAction)
{
    if(!actionHandler)  return;
    
    for(DAVA::int32 i = 0; i < ResourceEditor::VIEWPORT_COUNT; ++i)
    {
        if(viewportAction == viewportActions[i])
        {
            actionHandler->SetViewport((ResourceEditor::eViewportType)i);
        }
    }
}

