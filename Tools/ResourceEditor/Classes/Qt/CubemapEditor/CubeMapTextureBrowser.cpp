#include "CubemapEditor/CubeMapTextureBrowser.h"
#include "CubemapEditor/CubemapEditorDialog.h"
#include "CubemapEditor/CubemapUtils.h"
#include "Qt/Main/QtUtils.h"
#include "ui_CubeMapTextureBrowser.h"

#include <QFileDialog>
#include <QScrollBar>

#include "SceneEditor/EditorSettings.h"
#include <qdir>

const String CUBEMAP_LAST_PROJECT_DIR_KEY = "cubemap_last_proj_dir";

CubeMapTextureBrowser::CubeMapTextureBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CubeMapTextureBrowser)
{
    ui->setupUi(this);
	
	ui->listTextures->setItemDelegate(&cubeListItemDelegate);
	
	ConnectSignals();
	
	FilePath projectPath = CubemapUtils::GetDialogSavedPath(CUBEMAP_LAST_PROJECT_DIR_KEY,
															EditorSettings::Instance()->GetDataSourcePath().GetAbsolutePathname(),
															EditorSettings::Instance()->GetDataSourcePath().GetAbsolutePathname());
		
	ui->textRootPath->setText(projectPath.GetAbsolutePathname().c_str());
	ReloadTextures(projectPath.GetAbsolutePathname());
	
	UpdateCheckedState();
}

CubeMapTextureBrowser::~CubeMapTextureBrowser()
{
    delete ui;
}

void CubeMapTextureBrowser::ConnectSignals()
{    
	QObject::connect(ui->buttonSelectRootPath, SIGNAL(pressed()), this, SLOT(OnChooseDirectoryClicked()));
	QObject::connect(ui->buttonCreateCube, SIGNAL(pressed()), this, SLOT(OnCreateCubemapClicked()));
	QObject::connect(ui->buttonReload, SIGNAL(pressed()), this, SLOT(OnReloadClicked()));
	QObject::connect(&cubeListItemDelegate, SIGNAL(OnEditCubemap(const QModelIndex&)), this, SLOT(OnEditCubemap(const QModelIndex&)));
	QObject::connect(&cubeListItemDelegate, SIGNAL(OnItemCheckStateChanged(const QModelIndex&)), this, SLOT(OnItemCheckStateChanged(const QModelIndex&)));
	QObject::connect(ui->buttonRemove, SIGNAL(pressed()), this, SLOT(OnDeleteSelectedItemsClicked()));
}

void CubeMapTextureBrowser::ReloadTextures(const DAVA::String& rootPath)
{
	cubeListItemDelegate.ClearCache();
	ui->listTextures->clear();
	
	QDir dir(rootPath.c_str());
	QStringList filesList = dir.entryList(QStringList("*.tex"));
	int cubemapTextures = 0;
	
	if(filesList.size() > 0)
	{
		QStringList fullPathFileList;
		FilePath fp = rootPath;
		for(int i = 0; i < filesList.size(); ++i)
		{
			QString str = filesList.at(i);
			fp.ReplaceFilename(str.toStdString());
			
			DAVA::TextureDescriptor* texDesc = DAVA::TextureDescriptor::CreateFromFile(fp);
			if(texDesc && texDesc->IsCubeMap())
			{
				fullPathFileList.append(QString(fp.GetAbsolutePathname().c_str()));
			}
		}
		
		cubeListItemDelegate.UpdateCache(fullPathFileList);
		
		for(int i = 0; i < fullPathFileList.size(); ++i)
		{
			fp = fullPathFileList.at(i).toStdString();
			
			QListWidgetItem* listItem = new QListWidgetItem();
			listItem->setData(Qt::CheckStateRole, false);
			listItem->setData(CUBELIST_DELEGATE_ITEMFULLPATH, fullPathFileList.at(i));
			listItem->setData(CUBELIST_DELEGATE_ITEMFILENAME, fp.GetFilename().c_str());
			ui->listTextures->addItem(listItem);			
		}
		
		cubemapTextures = fullPathFileList.size();
		ui->listTextures->setCurrentItem(ui->listTextures->item(0));
	}
	
	ui->listTextures->setVisible(cubemapTextures > 0);
}

void CubeMapTextureBrowser::ReloadTexturesFromUI(QString& path)
{
	if(path.at(path.size() - 1) != QChar('/') &&
	   path.at(path.size() - 1) != QChar('\\'))
	{
		path += "/";
	}
	
	ui->textRootPath->setText(path);
	
	FilePath projectPath = path.toStdString();
	EditorSettings::Instance()->GetSettings()->SetString(CUBEMAP_LAST_PROJECT_DIR_KEY,
														 projectPath.GetAbsolutePathname());
	EditorSettings::Instance()->Save();
	
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
	QString newDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
													   ui->textRootPath->text());
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
													tr("Create Cubemap Texture"),
													ui->textRootPath->text(),
													tr("Tex File (*.tex)"));
	
	if(!fileName.isNull())
	{
		CubemapEditorDialog dlg(this);
		FilePath fp = fileName.toStdString();
		
		DAVA::FilePath rootPath = fp.GetDirectory();
		dlg.InitForCreating(fp, rootPath);
		dlg.exec();
		
		QString path = rootPath.GetAbsolutePathname().c_str();
		ui->textRootPath->setText(path);
		int currentRow = ui->listTextures->currentRow();
		ReloadTexturesFromUI(path);
		
		if(ui->listTextures->count() > 0 &&
		   currentRow < ui->listTextures->count())
		{
			RestoreListSelection(currentRow);
		}
	}	
}

void CubeMapTextureBrowser::OnEditCubemap(const QModelIndex &index)
{
	CubemapEditorDialog dlg(this);
	
	QListWidgetItem* item = ui->listTextures->item(index.row());
	
	DAVA::FilePath rootPath = ui->textRootPath->text().toStdString();
	FilePath fp = item->data(CUBELIST_DELEGATE_ITEMFULLPATH).toString().toStdString();
	dlg.InitForEditing(fp, rootPath);
	dlg.exec();

	int currentRow = ui->listTextures->currentRow();
	QString path = ui->textRootPath->text();
	ReloadTexturesFromUI(path);
	
	RestoreListSelection(currentRow);
}

void CubeMapTextureBrowser::OnItemCheckStateChanged(const QModelIndex &index)
{
	UpdateCheckedState();
}

void CubeMapTextureBrowser::UpdateCheckedState()
{
	int checkedItemCount = GetCheckedItemsCount();
	QString text = (checkedItemCount > 0) ? QString("%1 item(s) selected").arg(QString().setNum(checkedItemCount)) : QString("");
	ui->selectedItemsStatus->setText(text);

	ui->buttonRemove->setEnabled(checkedItemCount > 0);
}

int CubeMapTextureBrowser::GetCheckedItemsCount()
{
	int itemCount = ui->listTextures->count();
	int checkedItemCount = 0;
	for(int i = 0; i < itemCount; ++i)
	{
		QListWidgetItem* item = ui->listTextures->item(i);
		bool checkedState = item->data(Qt::CheckStateRole).toBool();
		
		if(checkedState)
		{
			checkedItemCount++;
		}
	}
	
	return checkedItemCount;
}

void CubeMapTextureBrowser::OnDeleteSelectedItemsClicked()
{
	int checkedItemCount = GetCheckedItemsCount();
	int answer = MB_FLAG_NO;
	if(checkedItemCount > 0)
	{
		QString text = QString("%1 item(s) will be deleted. Continue?").arg(QString().setNum(checkedItemCount));
		answer = ShowQuestion("Confirmation",
							  text.toStdString(),
							  MB_FLAG_YES | MB_FLAG_NO,
							  MB_FLAG_NO);
	}
	
	if(MB_FLAG_YES == answer)
	{
		DAVA::Vector<DAVA::String> failedToRemove;
		int itemCount = ui->listTextures->count();
		for(int i = 0; i < itemCount; ++i)
		{
			QListWidgetItem* item = ui->listTextures->item(i);
			bool checkedState = item->data(Qt::CheckStateRole).toBool();
			
			if(checkedState)
			{
				FilePath fp = item->data(CUBELIST_DELEGATE_ITEMFULLPATH).toString().toStdString();
				if(fp.Exists())
				{
					DAVA::Vector<DAVA::String> faceNames;
					CubemapUtils::GenerateFaceNames(fp.GetAbsolutePathname(), faceNames);
					for(int faceIndex = 0; faceIndex < faceNames.size(); ++faceIndex)
					{
						bool removeResult = QFile::remove(faceNames[faceIndex].c_str());
						if(!removeResult)
						{
							failedToRemove.push_back(faceNames[faceIndex]);
						}
					}
					
					bool removeResult = QFile::remove(fp.GetAbsolutePathname().c_str());
					if(!removeResult)
					{
						failedToRemove.push_back(fp.GetAbsolutePathname().c_str());
					}
				}
			}
		}
		
		if(failedToRemove.size() > 0)
		{
			DAVA::String fileList;
			int count = failedToRemove.size();
			for(int i = 0; i < count; ++i)
			{
				fileList += failedToRemove[i];
				fileList += "\n";
			}
			
			DAVA::String message = "Failed to remove the following files. Please delete them manually.\n";
			message += fileList;
			
			ShowErrorDialog(message);
		}

		QString path = ui->textRootPath->text();
		ReloadTexturesFromUI(path);
		UpdateCheckedState();
	}
}