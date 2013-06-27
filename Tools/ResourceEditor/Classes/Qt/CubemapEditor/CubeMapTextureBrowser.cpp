#include "CubemapEditor/CubeMapTextureBrowser.h"
#include "CubemapEditor/CubemapEditorDialog.h"
#include "ui_CubeMapTextureBrowser.h"

#include <QFileDialog>
#include <QScrollBar>

#include "SceneEditor/EditorSettings.h"
#include <qdir>

CubeMapTextureBrowser::CubeMapTextureBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CubeMapTextureBrowser)
{
    ui->setupUi(this);
	
	ui->listTextures->setItemDelegate(&cubeListItemDelegate);
	
	ConnectSignals();
	
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
	
	ui->textRootPath->setText(projectPath.GetAbsolutePathname().c_str());
	ReloadTextures(projectPath.GetAbsolutePathname());
}

CubeMapTextureBrowser::~CubeMapTextureBrowser()
{
    delete ui;
}

void CubeMapTextureBrowser::ConnectSignals()
{    
	QObject::connect(ui->buttonSelectRootPath, SIGNAL(pressed()), this, SLOT(OnChooseDirectoryClicked()));
	QObject::connect(ui->buttonReload, SIGNAL(pressed()), this, SLOT(OnReloadClicked()));
	QObject::connect(ui->buttonCreateCube, SIGNAL(pressed()), this, SLOT(OnCreateCubemapClicked()));
	QObject::connect(ui->listTextures, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OnListItemDoubleClicked(QListWidgetItem*)));
}

void CubeMapTextureBrowser::ReloadTextures(const DAVA::String& rootPath)
{
	ui->listTextures->clear();
	
	QDir dir(rootPath.c_str());
	QStringList filesList = dir.entryList(QStringList("*.tex"));
	
	if(filesList.size() > 0)
	{
		FilePath fp = rootPath;
		for(int i = 0; i < filesList.size(); ++i)
		{
			QString str = filesList.at(i);
			fp.ReplaceFilename(str.toStdString());
			
			DAVA::TextureDescriptor* texDesc = DAVA::TextureDescriptor::CreateFromFile(fp);
			
			if(texDesc && texDesc->IsCubeMap())
			{
				QListWidgetItem* listItem = new QListWidgetItem();
				listItem->setData(CUBELIST_DELEGATE_ITEMFULLPATH, QString(fp.GetAbsolutePathname().c_str()));
				listItem->setData(CUBELIST_DELEGATE_ITEMFILENAME, str);
				ui->listTextures->addItem(listItem);				
			}
		}
		
		ui->listTextures->setCurrentItem(ui->listTextures->item(0));
	}
}

void CubeMapTextureBrowser::ReloadTexturesFromUI(QString& path)
{
	if(path.at(path.size() - 1) != QChar('/') &&
	   path.at(path.size() - 1) != QChar('\\'))
	{
		path += "/";
	}
	
	ui->textRootPath->setText(path);
	
	ReloadTextures(path.toStdString());
}

void CubeMapTextureBrowser::RestoreListSelection(int currentRow)
{
	if(ui->listTextures->count() >= currentRow &&
	   currentRow >= 0)
	{
		QListWidgetItem* item = ui->listTextures->item(currentRow);
		ui->listTextures->scrollToItem(item);
		ui->listTextures->setCurrentItem(item);
	}
}

///////////////////////////////////////////////////////////////

void CubeMapTextureBrowser::OnChooseDirectoryClicked()
{
	QString newDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"));
	if(!newDir.isNull())
	{
		ReloadTexturesFromUI(newDir);
	}
}

void CubeMapTextureBrowser::OnReloadClicked()
{
	QString path = ui->textRootPath->text();
	
	ReloadTexturesFromUI(path);
}

void CubeMapTextureBrowser::OnCreateCubemapClicked()
{
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Create Cube Texture"),
													ui->textRootPath->text(),
													tr("Tex File (*.tex)"));
	
	if(!fileName.isNull())
	{
		CubemapEditorDialog dlg(this);
		FilePath fp = fileName.toStdString();
		dlg.InitForCreating(fp);
		dlg.exec();
		
		int currentRow = ui->listTextures->currentRow();
		QString path = ui->textRootPath->text();
		ReloadTexturesFromUI(path);
		
		RestoreListSelection(currentRow);
	}	
}

void CubeMapTextureBrowser::OnListItemDoubleClicked(QListWidgetItem* item)
{
	CubemapEditorDialog dlg(this);
	FilePath fp = item->data(CUBELIST_DELEGATE_ITEMFULLPATH).toString().toStdString();
	dlg.InitForEditing(fp);
	dlg.exec();

	int currentRow = ui->listTextures->currentRow();
	QString path = ui->textRootPath->text();
	ReloadTexturesFromUI(path);
	
	RestoreListSelection(currentRow);
}