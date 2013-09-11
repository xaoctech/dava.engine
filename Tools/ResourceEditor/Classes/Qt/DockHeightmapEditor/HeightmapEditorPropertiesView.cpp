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



#include "HeightmapEditorPropertiesView.h"
#include "ui_heightmapEditorProperties.h"

#include "Qt/Scene/SceneSignals.h"
#include "../Main/mainwindow.h"
#include "../../Commands2/HeightmapEditorCommands2.h"

#include "Qt/Scene/System/LandscapeEditorDrawSystem/HeightmapProxy.h"

#include <QMessageBox>

HeightmapEditorPropertiesView::HeightmapEditorPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::HeightmapEditorPropertiesView)
,	activeScene(NULL)
,	toolbarAction(NULL)
,	dockWidget(NULL)
{
	ui->setupUi(this);

	Init();
}

HeightmapEditorPropertiesView::~HeightmapEditorPropertiesView()
{
	delete ui;
}

void HeightmapEditorPropertiesView::Init()
{
	ui->sliderWidgetBrushSize->Init(ResourceEditor::HEIGHTMAP_EDITOR_BRUSH_SIZE_CAPTION.c_str(), false,
									DEF_BRUSH_MAX_SIZE, DEF_BRUSH_MIN_SIZE, DEF_BRUSH_MIN_SIZE);
	ui->sliderWidgetStrength->Init(ResourceEditor::HEIGHTMAP_EDITOR_STRENGTH_CAPTION.c_str(), true,
								   DEF_STRENGTH_MAX_VALUE, 0, 0);
	ui->sliderWidgetAverageStrength->Init(ResourceEditor::HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_CAPTION.c_str(), false,
										  DEF_AVERAGE_STRENGTH_MAX_VALUE, DEF_AVERAGE_STRENGTH_MIN_VALUE,
										  DEF_AVERAGE_STRENGTH_MIN_VALUE);

	InitBrushImages();
	toolbarAction = QtMainWindow::Instance()->GetUI()->actionHeightMapEditor;

	dockWidget = QtMainWindow::Instance()->GetUI()->dockHeightmapEditor;
	dockWidget->setFeatures(dockWidget->features() & ~(QDockWidget::DockWidgetClosable));

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(DropperHeightChanged(SceneEditor2*, double)),
			this, SLOT(SetDropperHeight(SceneEditor2*, double)));
	connect(SceneSignals::Instance(), SIGNAL(HeightmapEditorToggled(SceneEditor2*)),
			this, SLOT(HeightmapEditorToggled(SceneEditor2*)));

	connect(ui->sliderWidgetBrushSize, SIGNAL(ValueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(ui->sliderWidgetStrength, SIGNAL(ValueChanged(int)), this, SLOT(SetStrength(int)));
	connect(ui->sliderWidgetAverageStrength, SIGNAL(ValueChanged(int)), this, SLOT(SetAverageStrength(int)));

	connect(ui->radioAbsolute, SIGNAL(clicked()), this, SLOT(SetAbsoluteDrawing()));
	connect(ui->radioRelative, SIGNAL(clicked()), this, SLOT(SetRelativeDrawing()));
	connect(ui->radioAverage, SIGNAL(clicked()), this, SLOT(SetAverageDrawing()));
	connect(ui->radioAbsDrop, SIGNAL(clicked()), this, SLOT(SetAbsDropDrawing()));
	connect(ui->radioDropper, SIGNAL(clicked()), this, SLOT(SetDropper()));
	connect(ui->radioCopyPaste, SIGNAL(clicked()), this, SLOT(SetHeightmapCopyPaste()));

	connect(ui->comboBrushImage, SIGNAL(currentIndexChanged(int)), this, SLOT(SetToolImage(int)));
	connect(ui->checkboxHeightmap, SIGNAL(stateChanged(int)), this, SLOT(SetCopyPasteHeightmap(int)));
	connect(ui->checkboxTilemask, SIGNAL(stateChanged(int)), this, SLOT(SetCopyPasteTilemask(int)));
	connect(ui->editHeight, SIGNAL(editingFinished()), this, SLOT(HeightUpdatedManually()));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	SetWidgetsState(false);
}

void HeightmapEditorPropertiesView::InitBrushImages()
{
	QSize iconSize = ui->comboBrushImage->iconSize();
	iconSize = iconSize.expandedTo(QSize(32, 32));
	ui->comboBrushImage->setIconSize(iconSize);

	FilePath toolsPath(ResourceEditor::HEIGHTMAP_EDITOR_TOOLS_PATH.c_str());
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

void HeightmapEditorPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void HeightmapEditorPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	if (activeScene)
	{
		KeyedArchive* customProperties = activeScene->GetCustomProperties();
		if (customProperties)
		{
			customProperties->SetInt32(ResourceEditor::HEIGHTMAP_EDITOR_BRUSH_SIZE_MIN,
									   ui->sliderWidgetBrushSize->GetRangeMin());
			customProperties->SetInt32(ResourceEditor::HEIGHTMAP_EDITOR_BRUSH_SIZE_MAX,
									   ui->sliderWidgetBrushSize->GetRangeMax());
			customProperties->SetInt32(ResourceEditor::HEIGHTMAP_EDITOR_STRENGTH_MAX,
									   ui->sliderWidgetStrength->GetRangeMax());
			customProperties->SetInt32(ResourceEditor::HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MIN,
									   ui->sliderWidgetAverageStrength->GetRangeMin());
			customProperties->SetInt32(ResourceEditor::HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MAX,
									   ui->sliderWidgetAverageStrength->GetRangeMax());
		}
	}

	activeScene = NULL;
}

void HeightmapEditorPropertiesView::SetDropperHeight(SceneEditor2* scene, double height)
{
	if (!activeScene || activeScene != scene)
	{
		return;
	}

	ui->editHeight->setText(QString::number(height));
}

void HeightmapEditorPropertiesView::SetWidgetsState(bool enabled)
{
	toolbarAction->setChecked(enabled);

	ui->sliderWidgetBrushSize->setEnabled(enabled);
	ui->comboBrushImage->setEnabled(enabled);
	ui->radioAbsolute->setEnabled(enabled);
	ui->radioRelative->setEnabled(enabled);
	ui->radioAverage->setEnabled(enabled);
	ui->radioAbsDrop->setEnabled(enabled);
	ui->radioDropper->setEnabled(enabled);
	ui->radioCopyPaste->setEnabled(enabled);
	ui->sliderWidgetStrength->setEnabled(enabled);
	ui->sliderWidgetAverageStrength->setEnabled(enabled);
	ui->checkboxHeightmap->setEnabled(enabled);
	ui->checkboxTilemask->setEnabled(enabled);

	BlockAllSignals(!enabled);
}

void HeightmapEditorPropertiesView::BlockAllSignals(bool block)
{
	ui->sliderWidgetBrushSize->blockSignals(block);
	ui->comboBrushImage->blockSignals(block);
	ui->radioAbsolute->blockSignals(block);
	ui->radioRelative->blockSignals(block);
	ui->radioAverage->blockSignals(block);
	ui->radioAbsDrop->blockSignals(block);
	ui->radioDropper->blockSignals(block);
	ui->radioCopyPaste->blockSignals(block);
	ui->sliderWidgetStrength->blockSignals(block);
	ui->sliderWidgetAverageStrength->blockSignals(block);
	ui->checkboxHeightmap->blockSignals(block);
	ui->checkboxTilemask->blockSignals(block);
}

void HeightmapEditorPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->heightmapEditorSystem->IsLandscapeEditingEnabled();
	int32 brushSize = IntFromBrushSize(scene->heightmapEditorSystem->GetBrushSize());
	int32 strength = IntFromStrength(scene->heightmapEditorSystem->GetStrength());
	int32 averageStrength = IntFromAverageStrength(scene->heightmapEditorSystem->GetAverageStrength());
	int32 toolImage = scene->heightmapEditorSystem->GetToolImage();
	HeightmapEditorSystem::eHeightmapDrawType drawingType = scene->heightmapEditorSystem->GetDrawingType();
	bool copyPasteHeightmap = scene->heightmapEditorSystem->GetCopyPasteHeightmap();
	bool copyPasteTilemask = scene->heightmapEditorSystem->GetCopyPasteTilemask();

	int32 brushRangeMin = DEF_BRUSH_MIN_SIZE;
	int32 brushRangeMax = DEF_BRUSH_MAX_SIZE;
	int32 strRangeMax = DEF_STRENGTH_MAX_VALUE;
	int32 avStrRangeMin = DEF_AVERAGE_STRENGTH_MIN_VALUE;
	int32 avStrRangeMax = DEF_AVERAGE_STRENGTH_MAX_VALUE;

	KeyedArchive* customProperties = scene->GetCustomProperties();
	if (customProperties)
	{
		brushRangeMin = customProperties->GetInt32(ResourceEditor::HEIGHTMAP_EDITOR_BRUSH_SIZE_MIN,
												   DEF_BRUSH_MIN_SIZE);
		brushRangeMax = customProperties->GetInt32(ResourceEditor::HEIGHTMAP_EDITOR_BRUSH_SIZE_MAX,
												   DEF_BRUSH_MAX_SIZE);
		strRangeMax = customProperties->GetInt32(ResourceEditor::HEIGHTMAP_EDITOR_STRENGTH_MAX,
												 DEF_STRENGTH_MAX_VALUE);
		avStrRangeMin = customProperties->GetInt32(ResourceEditor::HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MIN,
												   DEF_AVERAGE_STRENGTH_MIN_VALUE);
		avStrRangeMax = customProperties->GetInt32(ResourceEditor::HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MAX,
												   DEF_AVERAGE_STRENGTH_MAX_VALUE);
	}

	SetWidgetsState(enabled);

	BlockAllSignals(true);

	ui->sliderWidgetBrushSize->SetRangeMin(brushRangeMin);
	ui->sliderWidgetBrushSize->SetRangeMax(brushRangeMax);
	ui->sliderWidgetBrushSize->SetValue(brushSize);

	ui->sliderWidgetStrength->SetRangeMax(strRangeMax);
	ui->sliderWidgetStrength->SetValue(strength);

	ui->sliderWidgetAverageStrength->SetRangeMin(avStrRangeMin);
	ui->sliderWidgetAverageStrength->SetRangeMax(avStrRangeMax);
	ui->sliderWidgetAverageStrength->SetValue(averageStrength);

	ui->comboBrushImage->setCurrentIndex(toolImage);
	ui->checkboxHeightmap->setChecked(copyPasteHeightmap);
	ui->checkboxTilemask->setChecked(copyPasteTilemask);

	UpdateRadioState(drawingType);

	dockWidget->setVisible(enabled);
	BlockAllSignals(!enabled);
}

void HeightmapEditorPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->heightmapEditorSystem->IsLandscapeEditingEnabled())
	{
		activeScene->Exec(new ActionDisableHeightmapEditor(activeScene));
	}
	else
	{
		activeScene->Exec(new ActionEnableHeightmapEditor(activeScene));
	}
}

void HeightmapEditorPropertiesView::SetBrushSize(int brushSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetBrushSize(BrushSizeFromInt(brushSize));
}

void HeightmapEditorPropertiesView::SetToolImage(int toolImage)
{
	if (!activeScene)
	{
		return;
	}

	QString s = ui->comboBrushImage->itemData(toolImage).toString();

	if (!s.isEmpty())
	{
		FilePath fp(s.toStdString());
		activeScene->heightmapEditorSystem->SetToolImage(fp, toolImage);
	}
}

void HeightmapEditorPropertiesView::SetRelativeDrawing()
{
	if (!activeScene)
	{
		return;
	}
	
	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_RELATIVE);
}

void HeightmapEditorPropertiesView::SetAverageDrawing()
{
	if (!activeScene)
	{
		return;
	}
	
	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_AVERAGE);
}

void HeightmapEditorPropertiesView::SetAbsoluteDrawing()
{
	if (!activeScene)
	{
		return;
	}
	
	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE);
}

void HeightmapEditorPropertiesView::SetAbsDropDrawing()
{
	if (!activeScene)
	{
		return;
	}
	
	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE_DROPPER);
}

void HeightmapEditorPropertiesView::SetDropper()
{
	if (!activeScene)
	{
		return;
	}
	
	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_DROPPER);
}

void HeightmapEditorPropertiesView::SetHeightmapCopyPaste()
{
	if (!activeScene)
	{
		return;
	}
	
	SetDrawingType(HeightmapEditorSystem::HEIGHTMAP_COPY_PASTE);
}

void HeightmapEditorPropertiesView::SetStrength(int strength)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetStrength(StrengthFromInt(strength));
}

void HeightmapEditorPropertiesView::SetAverageStrength(int averageStrength)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetAverageStrength(AverageStrengthFromInt(averageStrength));
}

void HeightmapEditorPropertiesView::SetCopyPasteHeightmap(int state)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetCopyPasteHeightmap(state == Qt::Checked);
}

void HeightmapEditorPropertiesView::SetCopyPasteTilemask(int state)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->heightmapEditorSystem->SetCopyPasteTilemask(state == Qt::Checked);
}

void HeightmapEditorPropertiesView::SetDrawingType(HeightmapEditorSystem::eHeightmapDrawType type)
{
	if (!activeScene)
	{
		return;
	}

	BlockAllSignals(true);
	UpdateRadioState(type);
	BlockAllSignals(false);

	activeScene->heightmapEditorSystem->SetDrawingType(type);
}


// these functions are designed to convert values from sliders in ui
// to the values suitable for heightmap editor system

// float32 StrengthFromInt(int32) - converts strength from UI value to system value
float32 HeightmapEditorPropertiesView::StrengthFromInt(int32 val)
{
	// no conversion needed now. this function is for future compatibiilty
	return (float32)val;
}

// int32 IntFromStrength(float32) - converts strength from system value to UI value
int32 HeightmapEditorPropertiesView::IntFromStrength(float32 strength)
{
	return (int32)strength;
}

// float32 AverageStrengthFromInt(int32) converts value, received from ui to the average strength value,
// understood by the heightmap editor system
float32 HeightmapEditorPropertiesView::AverageStrengthFromInt(int32 val)
{
	// average strength in heightmap editor is the real number in the range [0 .. 1.0] (by default, upper bound could be different)
	float32 averageStrength = (float32)val / DEF_AVERAGE_STRENGTH_MAX_VALUE;

	return averageStrength;
}

// float32 AverageStrengthFromInt(int32) converts average strength value from the system to the ui value
int32 HeightmapEditorPropertiesView::IntFromAverageStrength(float32 averageStrength)
{
	int32 val = (int32)(averageStrength * DEF_AVERAGE_STRENGTH_MAX_VALUE);

	return val;
}

// int32 BrushSizeFromInt(int32) converts brush size value from the ui to the system value
int32 HeightmapEditorPropertiesView::BrushSizeFromInt(int32 val)
{
	// height map size is differ from the landscape texture size.
	// so to unify brush size necessary to additionally scale brush size by (texture size / height map size) coefficient
	float32 heightmapSize = activeScene->landscapeEditorDrawSystem->GetHeightmapProxy()->Size();
	float32 textureSize = activeScene->landscapeEditorDrawSystem->GetTextureSize();
	float32 coef = ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF / (textureSize / heightmapSize);
	int32 brushSize = (int32)(val * coef);

	return brushSize;
}

// int32 IntFromBrushSize(int32) converts brush size value from the system to the ui value
int32 HeightmapEditorPropertiesView::IntFromBrushSize(int32 brushSize)
{
	float32 heightmapSize = activeScene->landscapeEditorDrawSystem->GetHeightmapProxy()->Size();
	float32 textureSize = activeScene->landscapeEditorDrawSystem->GetTextureSize();
	float32 coef = ResourceEditor::LANDSCAPE_BRUSH_SIZE_UI_TO_SYSTEM_COEF / (textureSize / heightmapSize);
	int32 val = (int32)(brushSize / coef);

	return val;
}

// end of convert functions ==========================

void HeightmapEditorPropertiesView::UpdateRadioState(HeightmapEditorSystem::eHeightmapDrawType type)
{
	ui->radioAbsolute->setChecked(false);
	ui->radioRelative->setChecked(false);
	ui->radioAverage->setChecked(false);
	ui->radioAbsDrop->setChecked(false);
	ui->radioDropper->setChecked(false);
	ui->radioCopyPaste->setChecked(false);

	switch (type)
	{
		case HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE:
			ui->radioAbsolute->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_RELATIVE:
			ui->radioRelative->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_AVERAGE:
			ui->radioAverage->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
			ui->radioAbsDrop->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_DROPPER:
			ui->radioDropper->setChecked(true);
			break;

		case HeightmapEditorSystem::HEIGHTMAP_COPY_PASTE:
			ui->radioCopyPaste->setChecked(true);
			break;

		default:
			break;
	}
}

void HeightmapEditorPropertiesView::HeightUpdatedManually()
{
	if (!activeScene)
	{
		return;
	}

	float32 height = (float32)ui->editHeight->text().toFloat();
	activeScene->heightmapEditorSystem->SetDropperHeight(height);
}

void HeightmapEditorPropertiesView::HeightmapEditorToggled(SceneEditor2* scene)
{
	if (!activeScene || activeScene != scene)
	{
		return;
	}

	if (activeScene->heightmapEditorSystem->IsLandscapeEditingEnabled())
	{
		SetWidgetsState(true);
		SetBrushSize(ui->sliderWidgetBrushSize->GetValue());
		SetStrength(ui->sliderWidgetStrength->GetValue());
		SetAverageStrength(ui->sliderWidgetAverageStrength->GetValue());
		SetToolImage(ui->comboBrushImage->currentIndex());
		SetCopyPasteHeightmap(ui->checkboxHeightmap->checkState());
		SetCopyPasteTilemask(ui->checkboxTilemask->checkState());
		dockWidget->show();
	}
	else
	{
		SetWidgetsState(false);
		dockWidget->hide();
	}
}
