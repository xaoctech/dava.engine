#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"

#include "HierarchyTreeController.h"
#include "rectpropertygridwidget.h"
#include "createplatformdlg.h"
#include "createplatformdlg.h"
#include "createscreendlg.h"
#include "fontmanagerdialog.h"
#include "Dialogs/localizationeditordialog.h"
#include "ItemsCommand.h"
#include "CommandsController.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include "FileSystem/FileSystem.h"

#include "ResourcesManageHelper.h"

#define SPIN_SCALE 10.f

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	screenChangeUpdate = false;
	
    ui->setupUi(this);
    
	ui->davaGlWidget->setFocus();
    
    if(DAVA::Core::Instance())
    {
        DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
        if(options)
        {
            QString titleStr(options->GetString("title", "UIEditor").c_str());
            
            this->setWindowTitle(titleStr);
        }
    }

	ui->scaleSlider->setMinimum((int)(ui->scaleSpin->minimum() * SPIN_SCALE));
	ui->scaleSlider->setMaximum((int)(ui->scaleSpin->maximum() * SPIN_SCALE));
	
	connect(ui->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved()));
	connect(ui->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved()));

	ui->menuView->addAction(ui->hierarchyDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->libraryDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->propertiesDockWidget->toggleViewAction());

    connect(ui->actionFontManager, SIGNAL(triggered()), this, SLOT(OnOpenFontManager()));
    connect(ui->actionLocalizationManager, SIGNAL(triggered()), this, SLOT(OnOpenLocalizationManager()));
    //Help contents dialog
    connect(ui->actionHelpContents, SIGNAL(triggered()), this, SLOT(OnShowHelpContents()));
   
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectCreated()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectClosed()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectLoaded()),
			this,
			SLOT(OnProjectCreated()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedScreenChanged(const HierarchyTreeScreenNode*)),
			this,
			SLOT(OnSelectedScreenChanged()));
	
	connect(ui->hierarchyDockWidget->widget(),
			SIGNAL(CreateNewScreen(HierarchyTreeNode::HIERARCHYTREENODEID)),
			this,
			SLOT(OnNewScreen(HierarchyTreeNode::HIERARCHYTREENODEID)));
	
	connect(ScreenWrapper::Instance(),
			SIGNAL(UpdateScaleRequest(float)),
			this,
			SLOT(OnUpdateScaleRequest(float)));
	
	connect(ScreenWrapper::Instance(),
			SIGNAL(UpdateScreenPositionRequest(const QPoint&)),
			this,
			SLOT(OnUpdateScreenPositionRequest(const QPoint&)));

	//HierarchyTreeController::Instance()->Load("/Users/adebt/Downloads/Project1/project1.uieditor");
	//HierarchyTreeController::Instance()->GetTree().Save(":asd");
	
	InitMenu();
}

MainWindow::~MainWindow()
{
    for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        SafeDelete(recentPojectActions[i]);
    }
	
    delete ui;
}

void MainWindow::OnUpdateScaleRequest(float scaleDelta)
{
	ui->scaleSpin->setValue(ui->scaleSpin->value() + scaleDelta);
}

void MainWindow::on_scaleSpin_valueChanged(double arg1)
{
	ui->scaleSlider->setValue(int(arg1 * SPIN_SCALE));
	ScreenWrapper::Instance()->SetScale((float)arg1);
	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::on_scaleSlider_valueChanged(int value)
{
	double spinValue = value / SPIN_SCALE;
	if (Abs(spinValue - ui->scaleSpin->value()) > 0.01)
		ui->scaleSpin->setValue(spinValue);
}

void MainWindow::OnSliderMoved()
{
	UpdateScreenPosition();
}

void MainWindow::resizeEvent(QResizeEvent * event)
{
	QMainWindow::resizeEvent(event);

	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::showEvent(QShowEvent * event)
{
	QMainWindow::showEvent(event);
	
	UpdateSliders();
	UpdateScreenPosition();
}

void MainWindow::OnSelectedScreenChanged()
{
	screenChangeUpdate = true;
	if (HierarchyTreeController::Instance()->GetActiveScreen())
	{
		ui->scaleSpin->setValue(HierarchyTreeController::Instance()->GetActiveScreen()->GetScale());

		UpdateSliders();
		ui->horizontalScrollBar->setValue(HierarchyTreeController::Instance()->GetActiveScreen()->GetPosX());
		ui->verticalScrollBar->setValue(HierarchyTreeController::Instance()->GetActiveScreen()->GetPosY());
	}
	
	screenChangeUpdate = false;
	
	UpdateScreenPosition();
}

void MainWindow::UpdateSliders()
{
	QRect widgetRect = ui->davaGlWidget->rect();
	QRect viewRect(0, 0, (int)ScreenWrapper::Instance()->GetWidth(), (int)ScreenWrapper::Instance()->GetHeight());
	int valueV = (viewRect.height() - widgetRect.height()) / 2;
	if (valueV < 0) valueV = 0;
	int valueH = (viewRect.width() - widgetRect.width()) / 2;
	if (valueH < 0) valueH = 0;
	ui->verticalScrollBar->setMinimum(0);
	ui->verticalScrollBar->setMaximum(valueV);
	ui->horizontalScrollBar->setMinimum(0);
	ui->horizontalScrollBar->setMaximum(valueH);
}

void MainWindow::UpdateScreenPosition()
{
	if (screenChangeUpdate)
		return;
	
	int valueV = ui->verticalScrollBar->value();
	int valueH = ui->horizontalScrollBar->value();
	ScreenWrapper::Instance()->SetViewPos(valueH, valueV, ui->davaGlWidget->rect());
}

void MainWindow::OnUpdateScreenPositionRequest(const QPoint& posDelta)
{
	ui->horizontalScrollBar->setValue(ui->horizontalScrollBar->value() + posDelta.x());
	ui->verticalScrollBar->setValue(ui->verticalScrollBar->value() + posDelta.y());
}

void MainWindow::OnOpenFontManager()
{
    FontManagerDialog *fontManagerDialog = new FontManagerDialog();
    if (fontManagerDialog->exec())
    {
        delete fontManagerDialog;
    }
}

void MainWindow::OnOpenLocalizationManager()
{
    LocalizationEditorDialog *localizationManagerDialog = new LocalizationEditorDialog();
    if (localizationManagerDialog->exec())
    {
        delete localizationManagerDialog;
    }
}

void MainWindow::OnShowHelpContents()
{
    //Get help contents file absolute path
    QString helpPath = "file:///";
    helpPath += ResourcesManageHelper::GetHelpContentsPath();
    //Open help file in default browser new window
    QDesktopServices::openUrl(QUrl(helpPath));
}

void MainWindow::InitMenu()
{
	ui->menuView->addAction(ui->hierarchyDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->libraryDockWidget->toggleViewAction());
	ui->menuView->addAction(ui->propertiesDockWidget->toggleViewAction());

	connect(ui->actionNew_project, SIGNAL(triggered()), this, SLOT(OnNewProject()));
	connect(ui->actionSave_project, SIGNAL(triggered()), this, SLOT(OnSaveProject()));
	connect(ui->actionSave_As_project, SIGNAL(triggered()), this, SLOT(OnSaveAsProject()));
	connect(ui->actionLoad_project, SIGNAL(triggered()), this, SLOT(OnLoadProject()));
	connect(ui->actionClose_project, SIGNAL(triggered()), this, SLOT(OnCloseProject()));

	connect(ui->actionNew_platform, SIGNAL(triggered()), this, SLOT(OnNewPlatform()));
	connect(ui->actionNew_screen, SIGNAL(triggered()), this, SLOT(OnNewScreen()));
	
	connect(ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(MenuFileWillShow()));
    connect(ui->menuFile, SIGNAL(triggered(QAction *)), this, SLOT(FileMenuTriggered(QAction *)));
	
	//Create empty actions for recent projects files
	for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        recentPojectActions[i] = new QAction(this);
        recentPojectActions[i]->setObjectName(QString::fromUtf8(Format("recentPojectActions[%d]", i)));
    }
	
	UpdateMenu();
}

void MainWindow::UpdateMenu()
{
	//ui->actionNew
	ui->actionSave_project->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionSave_As_project->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionClose_project->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionNew_platform->setEnabled(HierarchyTreeController::Instance()->GetTree().IsProjectCreated());
	ui->actionNew_screen->setEnabled(HierarchyTreeController::Instance()->GetTree().GetPlatforms().size());
}

void MainWindow::OnNewProject()
{
	if (!CloseProject())
		return;
	
	QString filename = QFileDialog::getSaveFileName(this,
													tr("New project"),
													GetDefaultDirectory(),
													tr("Documents (*.uiEditor)") );
    if (filename.isNull())
		return;
	
	if (!HierarchyTreeController::Instance()->NewProject(filename))
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while creating project"));
		msgBox.exec();
	}
}

void MainWindow::OnProjectCreated()
{
	UpdateMenu();

	// Release focus from Dava GL widget, so after the first click to it
	// it will lock the keyboard and will process events successfully.
	ui->hierarchyDockWidget->setFocus();
}

void MainWindow::OnNewPlatform()
{
	CreatePlatformDlg dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreatePlatformCommand* cmd = new CreatePlatformCommand(dlg.GetPlatformName(), Vector2(dlg.GetWidth(), dlg.GetHeight()));
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
		UpdateMenu();
	}
}

void MainWindow::OnNewScreen(HierarchyTreeNode::HIERARCHYTREENODEID id/* = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY*/)
{
	CreateScreenDlg dlg(this);
	dlg.SetDefaultPlatform(id);
	if (dlg.exec() == QDialog::Accepted)
	{
		CreateScreenCommand* cmd = new CreateScreenCommand(dlg.GetScreenName(), dlg.GetPlatformId());
		CommandsController::Instance()->ExecuteCommand(cmd);
		SafeRelease(cmd);
	}
}

void MainWindow::MenuFileWillShow()
{
	//Delete old list of recent project actions
	for(DAVA::int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
        if(recentPojectActions[i]->parentWidget())
        {
            ui->menuFile->removeAction(recentPojectActions[i]);
        }
    }
    //Get up to date count of recent project actions
    int32 projectCount = EditorSettings::Instance()->GetLastOpenedCount();
    if(projectCount > 0)
    {
        QList<QAction *> recentActions;
        for(int32 i = 0; i < projectCount; ++i)
        {
            recentPojectActions[i]->setText(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
            recentActions.push_back(recentPojectActions[i]);
        }
        //Insert recent project actions into file menu
        ui->menuFile->insertActions(ui->actionExit, recentActions);
        ui->menuFile->insertSeparator(ui->actionExit);
    }
}

void MainWindow::FileMenuTriggered(QAction *resentScene)
{
    for(int32 i = 0; i < EditorSettings::RECENT_FILES_COUNT; ++i)
    {
		//Check if user clicked on one of the recent project link
        if(resentScene == recentPojectActions[i])
        {
			//Close and save current project if any
			if (!CloseProject())
				return;
		
			QString filename = QString::fromStdString(EditorSettings::Instance()->GetLastOpenedFile(i));
			if (filename.isNull())
				return;

			if (!HierarchyTreeController::Instance()->Load(filename))
			{
				QMessageBox msgBox;
				msgBox.setText(tr("Error while loading project"));
				msgBox.exec();
			}
			return;
        }
    }
}

void MainWindow::OnSaveProject()
{
	QString path = HierarchyTreeController::Instance()->GetTree().GetActiveProjectPath();
	if (HierarchyTreeController::Instance()->Save(path))
	{
		//If project was successfully saved - we should save new project path
		//and add this project to recent files list
		UpdateProjectSettings(path);
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while saving project"));
		msgBox.exec();
	}
}

void MainWindow::OnSaveAsProject()
{
	QString filename = QFileDialog::getSaveFileName(this,
													tr("Save project as"),
													GetDefaultDirectory(),
													tr("Documents (*.uiEditor)") );
    if (filename.isNull())
		return;

	if (HierarchyTreeController::Instance()->Save(filename))
	{
		//If project was successfully saved with new name - we should save new project path
		//and add this project to recent files list
		UpdateProjectSettings(filename);
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while saving project"));
		msgBox.exec();
	}
}

void MainWindow::OnLoadProject()
{
	if (!CloseProject())
		return;
	
	QString filename = QFileDialog::getOpenFileName(this,
													tr("Load project"),
													GetDefaultDirectory(),
													tr("Documents (*.uiEditor)") );
    if (filename.isNull())
		return;

	if (HierarchyTreeController::Instance()->Load(filename))
	{
		//If project was successfully loaded - we should save project path
		//and add this project to recent files list
		UpdateProjectSettings(filename);
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Error while loading project"));
		msgBox.exec();
	}
}

void MainWindow::OnCloseProject()
{
	CloseProject();
}

bool MainWindow::CloseProject()
{
	if (!CommandsController::Instance()->IsLastChangeSaved())
	{
		int ret = QMessageBox::warning(this, qApp->applicationName(),
									   tr("The project has been modified.\n"
										  "Do you want to save your changes?"),
									   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
									   QMessageBox::Save);
		if (ret == QMessageBox::Cancel)
			return false;
		else if (ret == QMessageBox::Save)
			OnSaveProject();
	}
	
	HierarchyTreeController::Instance()->CloseProject();
	return true;
}

void MainWindow::UpdateProjectSettings(const QString& filename)
{
	//Add file to recent project files list
	EditorSettings::Instance()->AddLastOpenedFile(filename.toStdString());
	//Save current project directory path
	QDir projectDir = QFileInfo(filename).absoluteDir();
	EditorSettings::Instance()->SetProjectPath(projectDir.absolutePath().toStdString());
}

QString MainWindow::GetDefaultDirectory()
{
	QString defaultDir = QString::fromStdString(EditorSettings::Instance()->GetProjectPath());
	//If default directory path is not available in project settings - use current working path
	if ( defaultDir.isNull() || defaultDir.isEmpty() )
	{
  		defaultDir = QDir::currentPath();
	}
	return defaultDir;
}
