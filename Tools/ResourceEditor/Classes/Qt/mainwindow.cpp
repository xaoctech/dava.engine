#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Classes/Qt/QtMainWindowHandler.h"
#include "Classes/Qt/GUIState.h"
#include "Classes/SceneEditor/EditorSettings.h"
#include "Classes/Qt/SceneDataManager.h"

#include "Classes/Qt/PointerHolder.h"
#include "LibraryModel.h"

#include <QToolBar>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"
#include "../SceneEditor/EditorConfig.h"

#include <QApplication>
#include <QPixmap>


QtMainWindow::QtMainWindow(QWidget *parent)
    :   QMainWindow(parent)
    ,   ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->davaGLWidget->setFocus();
 
    qApp->installEventFilter(this);
    
    new QtMainWindowHandler(this);
    QtMainWindowHandler::Instance()->SetDefaultFocusWidget(ui->davaGLWidget);

    libraryModel = new LibraryModel(this);

    SceneDataManager::Instance()->SetSceneGraphView(ui->sceneGraphTree);
    SceneDataManager::Instance()->SetLibraryView(ui->libraryView);
    SceneDataManager::Instance()->SetLibraryModel(libraryModel);
    libraryModel->Activate(ui->libraryView);
    
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

	EditorConfig::Instance()->ParseConfig(EditorSettings::Instance()->GetProjectPath() + "EditorConfig.yaml");
    
    SetupMainMenu();
    SetupToolBar();
    SetupDockWidgets();
    SetupProjectPath();
    
    QtMainWindowHandler::Instance()->RegisterStatusBar(ui->statusBar);
    QtMainWindowHandler::Instance()->RestoreDefaultFocus();

	posSaver.Attach(this, __FUNCTION__);
}

QtMainWindow::~QtMainWindow()
{
	QtMainWindowHandler::Instance()->Release();
    
    GUIState::Instance()->Release();
    
    delete ui;

    SafeDelete(libraryModel);
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
    ui->menuView->insertAction(ui->actionRestoreViews, actionToolBar);
    ui->menuView->insertAction(actionToolBar, actionLibrary);
    ui->menuView->insertAction(actionLibrary, actionProperties);
	ui->menuView->insertAction(actionProperties, actionReferences);
    ui->menuView->insertAction(actionReferences, actionEntities);
    ui->menuView->insertAction(actionEntities, actionDataGraph);
    ui->menuView->insertAction(actionDataGraph, actionSceneGraph);
    ui->menuView->insertAction(actionSceneGraph, actionCustomColors);
	ui->menuView->insertAction(actionCustomColors, actionVisibilityCheckTool);
    ui->menuView->insertSeparator(ui->actionRestoreViews);
    ui->menuView->insertSeparator(actionToolBar);
    ui->menuView->insertSeparator(actionProperties);
    actionHandler->RegisterDockActions(ResourceEditor::HIDABLEWIDGET_COUNT,
                                       actionSceneGraph, actionDataGraph, actionEntities,
                                       actionProperties, actionLibrary, actionToolBar, actionReferences, actionCustomColors, actionVisibilityCheckTool);


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
    
    SetupCustomColorsDock();
	SetupVisibilityToolDock();

    connect(ui->btnRefresh, SIGNAL(clicked()), QtMainWindowHandler::Instance(), SLOT(RefreshSceneGraph()));
}

void QtMainWindow::SetupCustomColorsDock()
{
    QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
    connect(ui->buttonCustomColorsEnable, SIGNAL(clicked()), handler, SLOT(ToggleCustomColors()));

	ui->buttonCustomColorsSave->blockSignals(true);
	ui->sliderCustomColorBrushSize->blockSignals(true);
	ui->comboboxCustomColors->blockSignals(true);

    connect(ui->buttonCustomColorsSave, SIGNAL(clicked()), handler, SLOT(SaveTextureCustomColors()));
    connect(ui->sliderCustomColorBrushSize, SIGNAL(valueChanged(int)), handler, SLOT(ChangeBrushSizeCustomColors(int)));
    connect(ui->comboboxCustomColors, SIGNAL(currentIndexChanged(int)), handler, SLOT(ChangeColorCustomColors(int)));
	connect(ui->buttonCustomColorsLoad, SIGNAL(clicked()), handler, SLOT(LoadTextureCustomColors()));

	QtMainWindowHandler::Instance()->RegisterCustomColorsWidgets(
		ui->buttonCustomColorsEnable,
		ui->buttonCustomColorsSave,
		ui->sliderCustomColorBrushSize,
		ui->comboboxCustomColors,
		ui->buttonCustomColorsLoad);
    
    QSize iconSize = ui->comboboxCustomColors->iconSize();
    iconSize = iconSize.expandedTo(QSize(100, 0));
    ui->comboboxCustomColors->setIconSize(iconSize);
    
    Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues("LandscapeCustomColorsDescription");
	bool isEveryColorHasDescription = customColors.size() == customColorsDescription.size() ? true : false;
    for(size_t i = 0; i < customColors.size(); ++i)
    {
        QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);
        
        QImage image(iconSize, QImage::Format_ARGB32);
        image.fill(color);
        
        QPixmap pixmap(iconSize);
        pixmap.convertFromImage(image, Qt::ColorOnly);
        
        QIcon icon(pixmap);
		String description = isEveryColorHasDescription ? customColorsDescription[i] : "";
        ui->comboboxCustomColors->addItem(icon, description.c_str());
    }
    handler->SetCustomColorsWidgetsState(false);
}

void QtMainWindow::SetupVisibilityToolDock()
{
	QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	
	connect(ui->buttonVisibilityToolEnable, SIGNAL(clicked()), handler, SLOT(ToggleVisibilityTool()));

	ui->buttonVisibilityToolSave->blockSignals(true);
	ui->buttonVisibilityToolSetArea->blockSignals(true);
	ui->buttonVisibilityToolSetPoint->blockSignals(true);
	ui->sliderVisibilityToolAreaSize->blockSignals(true);
	
	connect(ui->buttonVisibilityToolSave,		SIGNAL(clicked()),
			handler,							SLOT(SaveTextureVisibilityTool()));
	connect(ui->buttonVisibilityToolSetArea,	SIGNAL(clicked()),
			handler,							SLOT(SetVisibilityAreaVisibilityTool()));
	connect(ui->buttonVisibilityToolSetPoint,	SIGNAL(clicked()),
			handler,							SLOT(SetVisibilityPointVisibilityTool()));
	connect(ui->sliderVisibilityToolAreaSize,	SIGNAL(valueChanged(int)),
			handler,							SLOT(ChangleAreaSizeVisibilityTool(int)));
	
	handler->RegisterWidgetsVisibilityTool(ui->buttonVisibilityToolEnable,
										   ui->buttonVisibilityToolSave,
										   ui->buttonVisibilityToolSetPoint,
										   ui->buttonVisibilityToolSetArea,
										   ui->sliderVisibilityToolAreaSize);
	
	handler->SetWidgetsStateVisibilityTool(false);
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


