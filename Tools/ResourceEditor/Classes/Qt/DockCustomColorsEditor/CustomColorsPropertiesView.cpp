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



#include "CustomColorsPropertiesView.h"
#include "ui_CustomColorsProperties.h"

#include "Main/QtUtils.h"
#include "Project/ProjectManager.h"
#include "../SceneEditor/EditorConfig.h"
#include "../Scene/SceneSignals.h"
#include "../Main/mainwindow.h"
#include "../../Commands2/CustomColorsCommands2.h"

#include <QFileDialog>
#include <QMessageBox>

CustomColorsPropertiesView::CustomColorsPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::CustomColorsPropertiesView)
,	activeScene(NULL)
,	toolbarAction(NULL)
,	dockWidget(NULL)
{
	ui->setupUi(this);

	Init();
}

CustomColorsPropertiesView::~CustomColorsPropertiesView()
{
	delete ui;
}

void CustomColorsPropertiesView::Init()
{
	ui->sliderWidgetBrushSize->Init(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_CAPTION.c_str(), false,
									DEF_BRUSH_MAX_SIZE, DEF_BRUSH_MIN_SIZE, DEF_BRUSH_MIN_SIZE);

	toolbarAction = QtMainWindow::Instance()->GetUI()->actionCustomColorsEditor;

	dockWidget = QtMainWindow::Instance()->GetUI()->dockCustomColorsEditor;
	dockWidget->setFeatures(dockWidget->features() & ~(QDockWidget::DockWidgetClosable));

	connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));

	connect(SceneSignals::Instance(), SIGNAL(CustomColorsTextureShouldBeSaved(SceneEditor2*)),
			this, SLOT(NeedSaveCustomColorsTexture(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(CustomColorsToggled(SceneEditor2*)),
			this, SLOT(CustomColorsToggled(SceneEditor2*)));

	connect(ui->sliderWidgetBrushSize, SIGNAL(ValueChanged(int)), this, SLOT(SetBrushSize(int)));
	connect(ui->comboColor, SIGNAL(currentIndexChanged(int)), this, SLOT(SetColor(int)));
	connect(ui->buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(ui->buttonLoadTexture, SIGNAL(clicked()), this, SLOT(LoadTexture()));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	SetWidgetsState(false);
}

void CustomColorsPropertiesView::InitColors()
{
	ui->comboColor->clear();

	QSize iconSize = ui->comboColor->iconSize();
	iconSize = iconSize.expandedTo(QSize(100, 0));
	ui->comboColor->setIconSize(iconSize);

	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues(ResourceEditor::CUSTOM_COLORS_PROPERTY_COLORS);
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues(ResourceEditor::CUSTOM_COLORS_PROPERTY_DESCRIPTION);
	for(size_t i = 0; i < customColors.size(); ++i)
	{
		QColor color = QColor::fromRgbF(customColors[i].r, customColors[i].g, customColors[i].b, customColors[i].a);

		QImage image(iconSize, QImage::Format_ARGB32);
		image.fill(color);

		QPixmap pixmap(iconSize);
		pixmap.convertFromImage(image, Qt::ColorOnly);

		QIcon icon(pixmap);
		String description = (i >= customColorsDescription.size()) ? "" : customColorsDescription[i];
		ui->comboColor->addItem(icon, description.c_str());
	}
}

void CustomColorsPropertiesView::ProjectOpened(const QString& path)
{
	InitColors();
}

void CustomColorsPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}
	
	if (activeScene->customColorsSystem->IsLandscapeEditingEnabled())
	{
		activeScene->Exec(new ActionDisableCustomColors(activeScene));
	}
	else
	{
		activeScene->Exec(new ActionEnableCustomColors(activeScene));
	}
}

void CustomColorsPropertiesView::SetWidgetsState(bool enabled)
{
	toolbarAction->setChecked(enabled);

	ui->buttonSaveTexture->setEnabled(enabled);
	ui->sliderWidgetBrushSize->setEnabled(enabled);
	ui->comboColor->setEnabled(enabled);
	ui->buttonLoadTexture->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void CustomColorsPropertiesView::BlockAllSignals(bool block)
{
	ui->buttonSaveTexture->blockSignals(block);
	ui->sliderWidgetBrushSize->blockSignals(block);
	ui->comboColor->blockSignals(block);
	ui->buttonLoadTexture->blockSignals(block);
}

void CustomColorsPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	KeyedArchive* ar = scene->GetCustomProperties();

	bool enabled = scene->customColorsSystem->IsLandscapeEditingEnabled();
	int32 brushSize = IntFromBrushSize(scene->customColorsSystem->GetBrushSize());
	int32 colorIndex = scene->customColorsSystem->GetColor();

	int32 brushRangeMin = DEF_BRUSH_MIN_SIZE;
	int32 brushRangeMax = DEF_BRUSH_MAX_SIZE;

	KeyedArchive* customProperties = scene->GetCustomProperties();
	if (customProperties)
	{
		brushRangeMin = customProperties->GetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MIN, DEF_BRUSH_MIN_SIZE);
		brushRangeMax = customProperties->GetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MAX, DEF_BRUSH_MAX_SIZE);
	}

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderWidgetBrushSize->SetRangeMin(brushRangeMin);
	ui->sliderWidgetBrushSize->SetRangeMax(brushRangeMax);
	ui->sliderWidgetBrushSize->SetValue(brushSize);
	ui->comboColor->setCurrentIndex(colorIndex);
	dockWidget->setVisible(enabled);
	BlockAllSignals(!enabled);
}

void CustomColorsPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void CustomColorsPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	if (activeScene)
	{
		KeyedArchive* customProperties = activeScene->GetCustomProperties();
		if (customProperties)
		{
			customProperties->SetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MIN,
									   ui->sliderWidgetBrushSize->GetRangeMin());
			customProperties->SetInt32(ResourceEditor::CUSTOM_COLORS_BRUSH_SIZE_MAX,
									   ui->sliderWidgetBrushSize->GetRangeMax());
		}
	}

	activeScene = NULL;
}

void CustomColorsPropertiesView::SetBrushSize(int brushSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->customColorsSystem->SetBrushSize(BrushSizeFromInt(brushSize));
}

void CustomColorsPropertiesView::SetColor(int color)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->customColorsSystem->SetColor(color);
}

void CustomColorsPropertiesView::SaveTexture()
{
	if (!activeScene)
	{
		return;
	}

	FilePath selectedPathname = activeScene->customColorsSystem->GetCurrentSaveFileName();
	if (selectedPathname.IsEmpty())
	{
		selectedPathname = activeScene->GetScenePath().GetDirectory();
	}

	QString filePath = QFileDialog::getSaveFileName(NULL, QString(ResourceEditor::CUSTOM_COLORS_SAVE_CAPTION.c_str()),
													QString(selectedPathname.GetAbsolutePathname().c_str()),
													QString(ResourceEditor::CUSTOM_COLORS_FILE_FILTER.c_str()));
	selectedPathname = PathnameToDAVAStyle(filePath);

	if (!selectedPathname.IsEmpty())
	{
		activeScene->customColorsSystem->SaveTexture(selectedPathname);
	}
}

void CustomColorsPropertiesView::LoadTexture()
{
	if (!activeScene)
	{
		return;
	}

	FilePath currentPath = activeScene->customColorsSystem->GetCurrentSaveFileName();
	if (currentPath.IsEmpty())
	{
		currentPath = activeScene->GetScenePath().GetDirectory();
	}

	FilePath selectedPathname = GetOpenFileName(ResourceEditor::CUSTOM_COLORS_LOAD_CAPTION,
												currentPath,
												ResourceEditor::CUSTOM_COLORS_FILE_FILTER);
	if(!selectedPathname.IsEmpty())
	{
		activeScene->customColorsSystem->LoadTexture(selectedPathname);
	}
}

void CustomColorsPropertiesView::NeedSaveCustomColorsTexture(SceneEditor2* scene)
{
	if (!activeScene)
	{
		return;
	}

	FilePath selectedPathname = scene->customColorsSystem->GetCurrentSaveFileName();
	if(!selectedPathname.IsEmpty())
	{
		activeScene->customColorsSystem->SaveTexture(selectedPathname);
	}
	else
	{
		SaveTexture();
	}
}

int32 CustomColorsPropertiesView::BrushSizeFromInt(int32 val)
{
	int32 brushSize = val * 10;
	
	return brushSize;
}

int32 CustomColorsPropertiesView::IntFromBrushSize(int32 brushSize)
{
	int32 val = brushSize / 10;
	
	return val;
}

void CustomColorsPropertiesView::CustomColorsToggled(SceneEditor2* scene)
{
	if (!activeScene || activeScene != scene)
	{
		return;
	}
	
	if (activeScene->customColorsSystem->IsLandscapeEditingEnabled())
	{
		SetWidgetsState(true);
		
		SetBrushSize(ui->sliderWidgetBrushSize->GetValue());
		SetColor(ui->comboColor->currentIndex());
		
		dockWidget->show();
	}
	else
	{
		SetWidgetsState(false);
		dockWidget->hide();
	}
}
