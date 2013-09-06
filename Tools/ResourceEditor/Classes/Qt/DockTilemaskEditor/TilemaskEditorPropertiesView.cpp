/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "TilemaskEditorPropertiesView.h"
#include "ui_tilemaskEditorProperties.h"

#include "../Scene/SceneEditor2.h"
#include "TextureBrowser/TextureConvertor.h"

#include "Qt/Scene/SceneSignals.h"
#include "../Main/mainwindow.h"
#include "../../Commands2/TilemaskEditorCommands.h"

#include <QMessageBox>

TilemaskEditorPropertiesView::TilemaskEditorPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::TilemaskEditorPropertiesView)
,	activeScene(NULL)
,	dockWidget(NULL)
{
	ui->setupUi(this);

	Init();
}

TilemaskEditorPropertiesView::~TilemaskEditorPropertiesView()
{
	delete ui;
}

void TilemaskEditorPropertiesView::Init()
{
	ui->sliderWidgetBrushSize->Init(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_CAPTION.c_str(), false,
									DEF_BRUSH_MAX_SIZE, DEF_BRUSH_MIN_SIZE, DEF_BRUSH_MIN_SIZE);
	ui->sliderWidgetStrength->Init(ResourceEditor::TILEMASK_EDITOR_STRENGTH_CAPTION.c_str(), false,
								   DEF_STRENGTH_MAX_VALUE, DEF_STRENGTH_MIN_VALUE, DEF_STRENGTH_MIN_VALUE);

	InitBrushImages();
	toolbarAction = QtMainWindow::Instance()->GetUI()->actionTileMapEditor;

	dockWidget = QtMainWindow::Instance()->GetUI()->dockTilemaskEditor;
	dockWidget->setFeatures(dockWidget->features() & ~(QDockWidget::DockWidgetClosable));

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(TilemaskEditorToggled(SceneEditor2*)),
			this, SLOT(TilemaskEditorToggled(SceneEditor2*)));

	connect(ui->sliderWidgetBrushSize, SIGNAL(ValueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(ui->sliderWidgetStrength, SIGNAL(ValueChanged(int)), this, SLOT(SetStrength(int)));

	connect(ui->comboBrushImage, SIGNAL(currentIndexChanged(int)), this, SLOT(SetToolImage(int)));
	connect(ui->comboTileTexture, SIGNAL(currentIndexChanged(int)), this, SLOT(SetDrawTexture(int)));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	SetWidgetsState(false);
}

void TilemaskEditorPropertiesView::InitBrushImages()
{
	QSize iconSize = ui->comboBrushImage->iconSize();
	iconSize = iconSize.expandedTo(QSize(32, 32));
	ui->comboBrushImage->setIconSize(iconSize);

	FilePath toolsPath(ResourceEditor::TILEMASK_EDITOR_TOOLS_PATH);
	FileList *fileList = new FileList(toolsPath);
	for(int32 iFile = 0; iFile < fileList->GetCount(); ++iFile)
	{
		String filename = fileList->GetFilename(iFile);
		if(fileList->GetPathname(iFile).IsEqualToExtension(".png"))
		{
			String fullname = fileList->GetPathname(iFile).GetAbsolutePathname();

			FilePath f = fileList->GetPathname(iFile);
			f.ReplaceExtension("");

			QString qFullname = QString::fromStdString(fullname);
			QIcon toolIcon(qFullname);
			ui->comboBrushImage->addItem(toolIcon, f.GetFilename().c_str(), QVariant(qFullname));
		}
	}
}

void TilemaskEditorPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void TilemaskEditorPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	if (activeScene)
	{
		KeyedArchive* customProperties = activeScene->GetCustomProperties();
		if (customProperties)
		{
			customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MIN,
									   (int32)ui->sliderWidgetBrushSize->GetRangeMin());
			customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MAX,
									   (int32)ui->sliderWidgetBrushSize->GetRangeMax());
			customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MIN,
									   (int32)ui->sliderWidgetStrength->GetRangeMin());
			customProperties->SetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MAX,
									   (int32)ui->sliderWidgetStrength->GetRangeMax());
		}
	}

	activeScene = NULL;
}

void TilemaskEditorPropertiesView::SetWidgetsState(bool enabled)
{
	toolbarAction->setChecked(enabled);

	ui->sliderWidgetBrushSize->setEnabled(enabled);
	ui->sliderWidgetStrength->setEnabled(enabled);
	ui->comboBrushImage->setEnabled(enabled);
	ui->comboTileTexture->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void TilemaskEditorPropertiesView::BlockAllSignals(bool block)
{
	ui->sliderWidgetBrushSize->blockSignals(block);
	ui->sliderWidgetStrength->blockSignals(block);
	ui->comboBrushImage->blockSignals(block);
	ui->comboTileTexture->blockSignals(block);
}

void TilemaskEditorPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->tilemaskEditorSystem->IsLandscapeEditingEnabled();
	int32 brushSize = IntFromBrushSize(scene->tilemaskEditorSystem->GetBrushSize());
	int32 strength = IntFromStrength(scene->tilemaskEditorSystem->GetStrength());
	uint32 tileTexture = scene->tilemaskEditorSystem->GetTileTextureIndex();
	int32 toolImage = scene->tilemaskEditorSystem->GetToolImage();

	int32 brushRangeMin = DEF_BRUSH_MIN_SIZE;
	int32 brushRangeMax = DEF_BRUSH_MAX_SIZE;
	int32 strRangeMin = DEF_STRENGTH_MIN_VALUE;
	int32 strRangeMax = DEF_STRENGTH_MAX_VALUE;

	KeyedArchive* customProperties = scene->GetCustomProperties();
	if (customProperties)
	{
		brushRangeMin = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MIN, DEF_BRUSH_MIN_SIZE);
		brushRangeMax = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_BRUSH_SIZE_MAX, DEF_BRUSH_MAX_SIZE);
		strRangeMin = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MIN, DEF_STRENGTH_MIN_VALUE);
		strRangeMax = customProperties->GetInt32(ResourceEditor::TILEMASK_EDITOR_STRENGTH_MAX, DEF_STRENGTH_MAX_VALUE);
	}

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderWidgetBrushSize->SetRangeMin(brushRangeMin);
	ui->sliderWidgetBrushSize->SetRangeMax(brushRangeMax);
	ui->sliderWidgetBrushSize->SetValue(brushSize);

	ui->sliderWidgetStrength->SetRangeMin(strRangeMin);
	ui->sliderWidgetStrength->SetRangeMax(strRangeMax);
	ui->sliderWidgetStrength->SetValue(strength);

	ui->comboTileTexture->setCurrentIndex(tileTexture);
	ui->comboBrushImage->setCurrentIndex(toolImage);
	dockWidget->setVisible(enabled);
	BlockAllSignals(!enabled);
}

void TilemaskEditorPropertiesView::UpdateTileTextures()
{
	if (!activeScene)
	{
		return;
	}

	ui->comboTileTexture->clear();

	QSize iconSize = ui->comboTileTexture->iconSize();
	iconSize = iconSize.expandedTo(QSize(150, 32));
	ui->comboTileTexture->setIconSize(iconSize);

	for (int32 i = 0; i < (int32)activeScene->tilemaskEditorSystem->GetTileTextureCount(); ++i)
	{
		Texture* tileTexture = activeScene->tilemaskEditorSystem->GetTileTexture(i);

		uint32 previewWidth = Min(tileTexture->GetWidth(), 150);
		uint32 previewHeight = Min(tileTexture->GetHeight(), 32);

		Image* tileImage = tileTexture->CreateImageFromMemory();
		tileImage->ResizeCanvas(previewWidth, previewHeight);

		QImage img = TextureConvertor::FromDavaImage(tileImage);
		QIcon icon = QIcon(QPixmap::fromImage(img));

		ui->comboTileTexture->addItem(icon, "");
	}
}

void TilemaskEditorPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->tilemaskEditorSystem->IsLandscapeEditingEnabled())
	{
		activeScene->Exec(new ActionDisableTilemaskEditor(activeScene));
	}
	else
	{
		activeScene->Exec(new ActionEnableTilemaskEditor(activeScene));
	}
}

void TilemaskEditorPropertiesView::SetBrushSize(int brushSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->tilemaskEditorSystem->SetBrushSize(BrushSizeFromInt(brushSize));
}

void TilemaskEditorPropertiesView::SetToolImage(int imageIndex)
{
	if (!activeScene)
	{
		return;
	}

	QString s = ui->comboBrushImage->itemData(imageIndex).toString();

	if (!s.isEmpty())
	{
		FilePath fp(s.toStdString());
		activeScene->tilemaskEditorSystem->SetToolImage(fp, imageIndex);
	}
}

void TilemaskEditorPropertiesView::SetStrength(int strength)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->tilemaskEditorSystem->SetStrength(StrengthFromInt(strength));
}

void TilemaskEditorPropertiesView::SetDrawTexture(int textureIndex)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->tilemaskEditorSystem->SetTileTexture(textureIndex);
}

float32 TilemaskEditorPropertiesView::StrengthFromInt(int32 val)
{
	float32 max = 2.0 * DEF_STRENGTH_MAX_VALUE;
	float32 strength = val / max;

	return strength;
}

int32 TilemaskEditorPropertiesView::IntFromStrength(float32 strength)
{
	int32 value = (int32)(strength * 2.f * DEF_STRENGTH_MAX_VALUE);

	return value;
}

int32 TilemaskEditorPropertiesView::BrushSizeFromInt(int32 val)
{
	int32 brushSize = val * 10;

	return brushSize;
}

int32 TilemaskEditorPropertiesView::IntFromBrushSize(int32 brushSize)
{
	int32 val = brushSize / 10;

	return val;
}

void TilemaskEditorPropertiesView::TilemaskEditorToggled(SceneEditor2* scene)
{
	if (!activeScene || activeScene != scene)
	{
		return;
	}

	if (activeScene->tilemaskEditorSystem->IsLandscapeEditingEnabled())
	{
		SetWidgetsState(true);
		UpdateTileTextures();
		SetBrushSize(ui->sliderWidgetBrushSize->GetValue());
		SetStrength(ui->sliderWidgetStrength->GetValue());
		SetToolImage(ui->comboBrushImage->currentIndex());
		SetDrawTexture(ui->comboTileTexture->currentIndex());
		dockWidget->show();
	}
	else
	{
		SetWidgetsState(false);
		dockWidget->hide();
	}
}
