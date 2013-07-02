#include "CubemapEditor/CubemapEditorDialog.h"
#include "CubemapEditor/ClickableQLabel.h"
#include "CubemapEditor/CubemapUtils.h"
#include "SceneEditor/EditorSettings.h"
#include "Qt/Main/QtUtils.h"
#include <QFileDialog>
#include "ui_cubemapeditordialog.h"

CubemapEditorDialog::CubemapEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CubemapEditorDialog)
{
    ui->setupUi(this);
	
	faceHeight = -1.0f;
	faceWidth = -1.0f;
	facePath = new QString[CubemapUtils::GetMaxFaces()];
	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		facePath[i] = QString::null;
	}
	
	ConnectSignals();
}

CubemapEditorDialog::~CubemapEditorDialog()
{
    delete ui;
	SafeDeleteArray(facePath);
}

void CubemapEditorDialog::ConnectSignals()
{
	QObject::connect(ui->labelPX, SIGNAL(OnLabelClicked()), this, SLOT(OnPXClicked()));
	QObject::connect(ui->labelNX, SIGNAL(OnLabelClicked()), this, SLOT(OnNXClicked()));
	QObject::connect(ui->labelPY, SIGNAL(OnLabelClicked()), this, SLOT(OnPYClicked()));
	QObject::connect(ui->labelNY, SIGNAL(OnLabelClicked()), this, SLOT(OnNYClicked()));
	QObject::connect(ui->labelPZ, SIGNAL(OnLabelClicked()), this, SLOT(OnPZClicked()));
	QObject::connect(ui->labelNZ, SIGNAL(OnLabelClicked()), this, SLOT(OnNZClicked()));
	
	//QObject::connect(ui->buttonLoadTexture, SIGNAL(pressed()), this, SLOT(OnLoadTexture()));
	QObject::connect(ui->buttonSave, SIGNAL(pressed()), this, SLOT(OnSave()));
	QObject::connect(ui->buttonClose, SIGNAL(pressed()), this, SLOT(OnClose()));
}

void CubemapEditorDialog::LoadImageFromUserFile(float rotation, int face)
{
	FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
	QString fileName = QFileDialog::getOpenFileName(this,
													tr("Open Face Image"),
													QString::fromStdString(projectPath.GetAbsolutePathname()),
													tr("Image Files (*.png)"));
	
	if(!fileName.isNull())
	{
		String stdFilePath = fileName.toStdString();
		LoadImageTo(stdFilePath, face);
	}
}

void CubemapEditorDialog::LoadImageTo(const DAVA::String& filePath, int face)
{
	ClickableQLabel* label = GetLabelForFace(face);
	
	QString fileName = filePath.c_str();
	
	QImage faceImage;
	faceImage.load(fileName);
	
	if(VerifyImage(faceImage))
	{
		QImage scaledFace = faceImage.scaled(label->width(), label->height());
		label->setPixmap(QPixmap::fromImage(scaledFace));
		
		facePath[face] = fileName;
		
		if(faceHeight < 0)
		{
			faceHeight = faceImage.height();
			faceWidth = faceImage.width();
			UpdateFaceInfo();
		}
		
		UpdateButtonState();
	}
	else
	{
		//TODO: display warning
	}

}

ClickableQLabel* CubemapEditorDialog::GetLabelForFace(int face)
{
	ClickableQLabel* labels[] =
	{
		ui->labelPX,
		ui->labelNX,
		ui->labelPY,
		ui->labelNY,
		ui->labelPZ,
		ui->labelNZ
	};
	
	return labels[face];
}

bool CubemapEditorDialog::VerifyImage(const QImage& image)
{
	return true;
}

void CubemapEditorDialog::UpdateFaceInfo()
{
	ui->labelFaceHeight->setText(QString().setNum((int)faceHeight));
	ui->labelFaceWidth->setText(QString().setNum((int)faceWidth));
}

void CubemapEditorDialog::UpdateButtonState()
{
	bool enableSave = false;
	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(QString::null != facePath[i])
		{
			enableSave = true;
			break;
		}
	}
	
	ui->buttonSave->setEnabled(enableSave);
}

bool CubemapEditorDialog::AnyFaceLoaded()
{
	bool faceLoaded = false;
	for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(QString::null != facePath[i])
		{
			faceLoaded = true;
			break;
		}
	}

	return faceLoaded;
}

void CubemapEditorDialog::LoadCubemap(const QString& path)
{
	FilePath filePath(path.toStdString());
	TextureDescriptor* texDescriptor = TextureDescriptor::CreateFromFile(filePath);
	
	if(NULL != texDescriptor &&
	   texDescriptor->IsCubeMap())
	{
		String fileNameWithoutExtension = filePath.GetFilename();
		String extension = filePath.GetExtension();
		fileNameWithoutExtension.replace(fileNameWithoutExtension.find(extension), extension.size(), "");

		for(int i = 0; i < CubemapUtils::GetMaxFaces(); ++i)
		{
			if(texDescriptor->faceDescription & (1 << CubemapUtils::MapUIToFrameworkFace(i)))
			{
				FilePath faceFilePath = filePath;
				faceFilePath.ReplaceFilename(fileNameWithoutExtension +
											 CubemapUtils::GetFaceNameSuffix(CubemapUtils::MapUIToFrameworkFace(i)) + "." +
											 CubemapUtils::GetDefaultFaceExtension());

				LoadImageTo(faceFilePath.GetAbsolutePathname(), i);
			}
		}
	}
	else
	{
		if(NULL == texDescriptor)
		{
			ShowErrorDialog("Failed to load cubemap texture " + path.toStdString());
		}
		else
		{
			ShowErrorDialog("Failed to load cubemap texture " + path.toStdString() + ". Seems this is not a cubemap texture.");
		}
	}
}

void CubemapEditorDialog::SaveCubemap(const QString& path)
{
	FilePath filePath(path.toStdString());
	DAVA::uint8 faceMask = GetFaceMask();
	
	//copy file to the location where .tex will be put. Add suffixes to file names to distinguish faces
	String fileNameWithoutExtension = filePath.GetFilename();
	String extension = filePath.GetExtension();
	fileNameWithoutExtension.replace(fileNameWithoutExtension.find(extension), extension.size(), "");
	for(int i = 0 ; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(!facePath[i].isNull())
		{
			FilePath faceFilePath = filePath;
			faceFilePath.ReplaceFilename(fileNameWithoutExtension +
										 CubemapUtils::GetFaceNameSuffix(CubemapUtils::MapUIToFrameworkFace(i)) + "." +
										 CubemapUtils::GetDefaultFaceExtension());

			if(facePath[i] != faceFilePath.GetAbsolutePathname().c_str())
			{
				if(QFile::exists(faceFilePath.GetAbsolutePathname().c_str()))
				{
					int answer = ShowQuestion("File overwrite",
											  "File " + faceFilePath.GetAbsolutePathname() + " already exist. Do you want to overwrite it with " + facePath[i].toStdString(),
											  MB_FLAG_YES | MB_FLAG_NO, MB_FLAG_NO);
					
					if(MB_FLAG_YES == answer)
					{
						bool removeResult = QFile::remove(faceFilePath.GetAbsolutePathname().c_str());
						
						if(!removeResult)
						{
							ShowErrorDialog("Failed to copy texture " + facePath[i].toStdString() + " to " + faceFilePath.GetAbsolutePathname().c_str());
							return;
						}

					}
					else
					{
						continue;
					}
				}
				
				bool copyResult = QFile::copy(facePath[i], faceFilePath.GetAbsolutePathname().c_str());
				
				if(!copyResult)
				{
					ShowErrorDialog("Failed to copy texture " + facePath[i].toStdString() + " to " + faceFilePath.GetAbsolutePathname().c_str());
					return;
				}
			}
		}
	}
	
	TextureDescriptor* descriptor = new TextureDescriptor();
	descriptor->settings.wrapModeS = descriptor->settings.wrapModeT = Texture::WRAP_CLAMP_TO_EDGE;
    descriptor->settings.generateMipMaps = true;
	descriptor->settings.minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
	descriptor->settings.magFilter = Texture::FILTER_LINEAR;
    descriptor->exportedAsGpuFamily = GPU_UNKNOWN;
	descriptor->exportedAsPixelFormat = FORMAT_INVALID;
	descriptor->faceDescription = faceMask;
	    
    descriptor->Save(filePath);
	SafeRelease(descriptor);
	
	QMessageBox::information(this, "Cubemap texture save result", "Cubemap texture was saved successfully!");
}

DAVA::uint8 CubemapEditorDialog::GetFaceMask()
{
	DAVA::uint8 mask = 0;
	for(int i = 0 ; i < CubemapUtils::GetMaxFaces(); ++i)
	{
		if(!facePath[i].isNull())
		{
			mask |= 1 << CubemapUtils::MapUIToFrameworkFace(i);
		}
	}
	
	return mask;
}

void CubemapEditorDialog::InitForEditing(DAVA::FilePath& textureDescriptorPath)
{
	targetFile = textureDescriptorPath;
	editorMode = CubemapEditorDialog::eEditorModeEditing;
	
	LoadCubemap(targetFile.GetAbsolutePathname().c_str());
}

void CubemapEditorDialog::InitForCreating(DAVA::FilePath& textureDescriptorPath)
{
	targetFile = textureDescriptorPath;
	editorMode = CubemapEditorDialog::eEditorModeCreating;
}

////////////////////////////////////////////////////

void CubemapEditorDialog::OnPXClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_PX);
}

void CubemapEditorDialog::OnNXClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_NX);
}

void CubemapEditorDialog::OnPYClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_PY);
}

void CubemapEditorDialog::OnNYClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_NY);
}

void CubemapEditorDialog::OnPZClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_PZ);
}

void CubemapEditorDialog::OnNZClicked()
{
	LoadImageFromUserFile(0, CUBEMAPEDITOR_FACE_NZ);
}

void CubemapEditorDialog::OnLoadTexture()
{
	int answer = MB_FLAG_YES;
	if(AnyFaceLoaded())
	{
		int answer = ShowQuestion("Warning",
								  "Do you really want to load new cube texture over currently loaded one?",
								  MB_FLAG_YES | MB_FLAG_NO,
								  MB_FLAG_NO);		
	}
	
	if(MB_FLAG_YES == answer)
	{
		FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
		QString fileName = QFileDialog::getOpenFileName(this,
														tr("Open Cube Texture"),
														QString::fromStdString(projectPath.GetAbsolutePathname()),
														tr("Tex File (*.tex)"));
		
		if(!fileName.isNull())
		{
			LoadCubemap(fileName);
		}
	}
}

void CubemapEditorDialog::OnSave()
{
	/*FilePath projectPath = EditorSettings::Instance()->GetProjectPath();
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save Cube Texture"),
													QString::fromStdString(projectPath.GetAbsolutePathname()),
													tr("Tex File (*.tex)"));
	
	if(!fileName.isNull())
	{
		SaveCubemap(fileName);
	}*/
	
	SaveCubemap(targetFile.GetAbsolutePathname().c_str());
	
	close();
}

void CubemapEditorDialog::OnClose()
{
	close();
}

