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
    
    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        resentSceneActions[i] = new QAction(this);
        resentSceneActions[i]->setObjectName(QString::fromUtf8(DAVA::Format("resentSceneActions[%d]", i)));
    }

    SetupProjectPath();
}

MainWindow::~MainWindow()
{
    for(DAVA::int32 i = 0; i < EditorSettings::RESENT_FILES_COUNT; ++i)
    {
        DAVA::SafeDelete(resentSceneActions[i]);
    }
    
    
    DAVA::SafeDelete(actionHandler);
    delete ui;
}

void MainWindow::SetupMainMenu()
{
    connect(ui->actionNewScene, SIGNAL(triggered()), actionHandler, SLOT(NewScene()));
    connect(ui->actionOpenScene, SIGNAL(triggered()), actionHandler, SLOT(OpenScene()));
    connect(ui->actionOpenProject, SIGNAL(triggered()), actionHandler, SLOT(OpenProject()));
    //Resent files
    connect(ui->actionSaveScene, SIGNAL(triggered()), actionHandler, SLOT(SaveScene()));
    connect(ui->actionPNG, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPNG()));
    connect(ui->actionPVR, SIGNAL(triggered()), actionHandler, SLOT(ExportAsPVR()));
    connect(ui->actionDXT, SIGNAL(triggered()), actionHandler, SLOT(ExportAsDXT()));

    
    connect(ui->actionLandscape, SIGNAL(triggered()), actionHandler, SLOT(CreateLandscape()));
    connect(ui->actionLight, SIGNAL(triggered()), actionHandler, SLOT(CreateLight()));
    connect(ui->actionServiceNode, SIGNAL(triggered()), actionHandler, SLOT(CreateServiceNode()));
    connect(ui->actionBox, SIGNAL(triggered()), actionHandler, SLOT(CreateBox()));
    connect(ui->actionSphere, SIGNAL(triggered()), actionHandler, SLOT(CreateSphere()));
    connect(ui->actionCamera, SIGNAL(triggered()), actionHandler, SLOT(CreateCamera()));
    connect(ui->actionImposter, SIGNAL(triggered()), actionHandler, SLOT(CreateImposter()));
    connect(ui->actionUserNode, SIGNAL(triggered()), actionHandler, SLOT(CreateUserNode()));

    connect(ui->actionMaterialEditor, SIGNAL(triggered()), actionHandler, SLOT(Materials()));
    connect(ui->actionTextureConverter, SIGNAL(triggered()), actionHandler, SLOT(ConvertTextures()));
    connect(ui->actionHeightMapEditor, SIGNAL(triggered()), actionHandler, SLOT(HeightmapEditor()));
    connect(ui->actionTileMapEditor, SIGNAL(triggered()), actionHandler, SLOT(TilemapEditor()));
    
    connect(ui->actionIPhone, SIGNAL(triggered()), actionHandler, SLOT(ViewportiPhone()));
    connect(ui->actionRetina, SIGNAL(triggered()), actionHandler, SLOT(VeiwportRetina()));
    connect(ui->actionIPad, SIGNAL(triggered()), actionHandler, SLOT(ViewportiPad()));
    connect(ui->actionDefault, SIGNAL(triggered()), actionHandler, SLOT(ViewportDefault()));
    
    connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->menuResentScenes, SIGNAL(triggered(QAction *)), this, SLOT(ResentSceneTriggered(QAction *)));
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