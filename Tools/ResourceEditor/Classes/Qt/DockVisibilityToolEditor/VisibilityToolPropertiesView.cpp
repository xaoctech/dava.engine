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



#include "VisibilityToolPropertiesView.h"
#include "ui_VisibilityToolPropertiesView.h"

#include "Main/QtUtils.h"
#include "../Scene/SceneSignals.h"
#include "../Scene/SceneEditor2.h"
#include "../Main/mainwindow.h"
#include "../../Commands2/VisibilityToolActions.h"

#include <QFileDialog>

VisibilityToolPropertiesView::VisibilityToolPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::VisibilityToolPropertiesView)
,	activeScene(NULL)
,	toolbarAction(NULL)
,	dockWidget(NULL)
{
	ui->setupUi(this);

	Init();
}

VisibilityToolPropertiesView::~VisibilityToolPropertiesView()
{
	delete ui;
}

void VisibilityToolPropertiesView::Init()
{
	ui->sliderWidgetAreaSize->Init(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_CAPTION.c_str(), false,
								   DEF_AREA_MAX_SIZE, DEF_AREA_MIN_SIZE, DEF_AREA_MIN_SIZE);

	toolbarAction = QtMainWindow::Instance()->GetUI()->actionVisibilityCheckTool;

	dockWidget = QtMainWindow::Instance()->GetUI()->dockVisibilityToolEditor;
	dockWidget->setFeatures(dockWidget->features() & ~(QDockWidget::DockWidgetClosable));

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(VisibilityToolStateChanged(SceneEditor2*, VisibilityToolSystem::eVisibilityToolState)),
			this, SLOT(SetVisibilityToolButtonsState(SceneEditor2*, VisibilityToolSystem::eVisibilityToolState)));
	connect(SceneSignals::Instance(), SIGNAL(VisibilityToolToggled(SceneEditor2*)),
			this, SLOT(VisibilityToolToggled(SceneEditor2*)));

	connect(ui->buttonSaveTexture, SIGNAL(clicked()), this, SLOT(SaveTexture()));
	connect(ui->buttonSetVisibilityPoint, SIGNAL(clicked()), this, SLOT(SetVisibilityPoint()));
	connect(ui->buttonSetVisibilityArea, SIGNAL(clicked()), this, SLOT(SetVisibilityArea()));
	connect(ui->sliderWidgetAreaSize, SIGNAL(ValueChanged(int)), this, SLOT(SetVisibilityAreaSize(int)));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	SetWidgetsState(false);
}

void VisibilityToolPropertiesView::SetWidgetsState(bool enabled)
{
	toolbarAction->setChecked(enabled);

	ui->buttonSaveTexture->setEnabled(enabled);
	ui->buttonSetVisibilityPoint->setEnabled(enabled);
	ui->buttonSetVisibilityArea->setEnabled(enabled);
	ui->sliderWidgetAreaSize->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void VisibilityToolPropertiesView::BlockAllSignals(bool block)
{
	ui->buttonSaveTexture->blockSignals(block);
	ui->buttonSetVisibilityPoint->blockSignals(block);
	ui->buttonSetVisibilityArea->blockSignals(block);
	ui->sliderWidgetAreaSize->blockSignals(block);
}

void VisibilityToolPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void VisibilityToolPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	if (activeScene)
	{
		KeyedArchive* customProperties = activeScene->GetCustomProperties();
		if (customProperties)
		{
			customProperties->SetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MIN,
									   ui->sliderWidgetAreaSize->GetRangeMin());
			customProperties->SetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MAX,
									   ui->sliderWidgetAreaSize->GetRangeMax());
		}
	}

	activeScene = NULL;
}

void VisibilityToolPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->visibilityToolSystem->IsLandscapeEditingEnabled();
	int32 brushSize = IntFromAreaSize(scene->visibilityToolSystem->GetBrushSize());

	int32 areaSizeMin = DEF_AREA_MIN_SIZE;
	int32 areaSizeMax = DEF_AREA_MAX_SIZE;

	KeyedArchive* customProperties = scene->GetCustomProperties();
	if (customProperties)
	{
		areaSizeMin = customProperties->GetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MIN, DEF_AREA_MIN_SIZE);
		areaSizeMax = customProperties->GetInt32(ResourceEditor::VISIBILITY_TOOL_AREA_SIZE_MAX, DEF_AREA_MAX_SIZE);
	}

	SetWidgetsState(enabled);

	BlockAllSignals(true);
	ui->sliderWidgetAreaSize->SetRangeMin(areaSizeMin);
	ui->sliderWidgetAreaSize->SetRangeMax(areaSizeMax);
	ui->sliderWidgetAreaSize->SetValue(brushSize);
	dockWidget->setVisible(enabled);
	BlockAllSignals(!enabled);
}

void VisibilityToolPropertiesView::SetVisibilityToolButtonsState(SceneEditor2* scene,
																 VisibilityToolSystem::eVisibilityToolState state)
{
	if (!activeScene || scene != activeScene)
	{
		return;
	}

	bool pointButton = false;
	bool areaButton = false;

	switch (state)
	{
		case VisibilityToolSystem::VT_STATE_SET_AREA:
			areaButton = true;
			break;

		case VisibilityToolSystem::VT_STATE_SET_POINT:
			pointButton = true;
			break;

		default:
			break;
	}
	bool b;

	b = ui->buttonSetVisibilityPoint->signalsBlocked();
	ui->buttonSetVisibilityPoint->blockSignals(true);
	ui->buttonSetVisibilityPoint->setChecked(pointButton);
	ui->buttonSetVisibilityPoint->blockSignals(b);

	b = ui->buttonSetVisibilityArea->signalsBlocked();
	ui->buttonSetVisibilityArea->blockSignals(true);
	ui->buttonSetVisibilityArea->setChecked(areaButton);
	ui->buttonSetVisibilityArea->blockSignals(b);
}

void VisibilityToolPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	if (activeScene->visibilityToolSystem->IsLandscapeEditingEnabled())
	{
		activeScene->Exec(new ActionDisableVisibilityTool(activeScene));
	}
	else
	{
		activeScene->Exec(new ActionEnableVisibilityTool(activeScene));
	}
}

void VisibilityToolPropertiesView::SaveTexture()
{
	if (!activeScene)
	{
		return;
	}

	FilePath currentPath = FileSystem::Instance()->GetUserDocumentsPath();
	QString filePath = QFileDialog::getSaveFileName(NULL,
													QString(ResourceEditor::VISIBILITY_TOOL_SAVE_CAPTION.c_str()),
													QString(currentPath.GetAbsolutePathname().c_str()),
													QString(ResourceEditor::VISIBILITY_TOOL_FILE_FILTER.c_str()));

	FilePath selectedPathname = PathnameToDAVAStyle(filePath);

	if(!selectedPathname.IsEmpty())
	{
		activeScene->visibilityToolSystem->SaveTexture(selectedPathname);
	}
}

void VisibilityToolPropertiesView::SetVisibilityPoint()
{
	if (!activeScene)
	{
		return;
	}

	activeScene->visibilityToolSystem->SetVisibilityPoint();
}

void VisibilityToolPropertiesView::SetVisibilityArea()
{
	if (!activeScene)
	{
		return;
	}

	activeScene->visibilityToolSystem->SetVisibilityArea();
}

void VisibilityToolPropertiesView::SetVisibilityAreaSize(int areaSize)
{
	if (!activeScene)
	{
		return;
	}

	activeScene->visibilityToolSystem->SetBrushSize(AreaSizeFromInt(areaSize));
}

int32 VisibilityToolPropertiesView::AreaSizeFromInt(int32 val)
{
	int32 areaSize = val * 10;

	return areaSize;
}

int32 VisibilityToolPropertiesView::IntFromAreaSize(int32 areaSize)
{
	int32 val = areaSize / 10;

	return val;
}

void VisibilityToolPropertiesView::VisibilityToolToggled(SceneEditor2* scene)
{
	if (!activeScene || activeScene != scene)
	{
		return;
	}
	
	if (activeScene->visibilityToolSystem->IsLandscapeEditingEnabled())
	{
		SetWidgetsState(true);
		SetVisibilityAreaSize(ui->sliderWidgetAreaSize->GetValue());
		dockWidget->show();
	}
	else
	{
		SetWidgetsState(false);
		dockWidget->hide();
	}
}
