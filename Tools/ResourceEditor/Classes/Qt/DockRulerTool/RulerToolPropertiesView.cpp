/*==================================================================================
 Copyright (c) 2008, DAVA, INC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "RulerToolPropertiesView.h"
#include "ui_RulerToolPropertiesView.h"

#include "Main/QtUtils.h"
#include "Project/ProjectManager.h"
#include "../SceneEditor/EditorConfig.h"
#include "../Scene/SceneSignals.h"
#include "../Main/mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

RulerToolPropertiesView::RulerToolPropertiesView(QWidget* parent)
:	QWidget(parent)
,	ui(new Ui::RulerToolPropertiesView)
,	activeScene(NULL)
,	toolbarAction(NULL)
,	dockWidget(NULL)
{
	ui->setupUi(this);

	Init();
}

RulerToolPropertiesView::~RulerToolPropertiesView()
{
	delete ui;
}

void RulerToolPropertiesView::Init()
{
	GLint range[2];
	glGetIntegerv(GL_LINE_WIDTH_RANGE, range);
	defLineWidthMinValue = range[0];
	defLineWidthMaxValue = range[1];

	ui->sliderWidgetLineWidth->Init(ResourceEditor::RULER_TOOL_LINE_WIDTH_CAPTION.c_str(), false,
									defLineWidthMaxValue, defLineWidthMinValue, defLineWidthMinValue);
	ui->sliderWidgetLineWidth->SetRangeChangingBlocked(true);

	toolbarAction = QtMainWindow::Instance()->GetUI()->actionRulerTool;
	dockWidget = QtMainWindow::Instance()->GetUI()->dockRulerTool;

	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
	connect(ui->sliderWidgetLineWidth, SIGNAL(ValueChanged(int)), this, SLOT(SetLineWidth(int)));

	connect(toolbarAction, SIGNAL(triggered()), this, SLOT(Toggle()));

	connect(SceneSignals::Instance(), SIGNAL(RulerToolLengthChanged(SceneEditor2*, double, double)),
			this, SLOT(UpdateLengths(SceneEditor2*, double, double)));

	SetWidgetsState(false);
}

void RulerToolPropertiesView::SetWidgetsState(bool enabled)
{
	toolbarAction->setChecked(enabled);

	ui->labelLength->setEnabled(enabled);
	ui->labelPreview->setEnabled(enabled);
	ui->sliderWidgetLineWidth->setEnabled(enabled);
	BlockAllSignals(!enabled);
}

void RulerToolPropertiesView::BlockAllSignals(bool block)
{
	ui->sliderWidgetLineWidth->blockSignals(block);
}

void RulerToolPropertiesView::UpdateFromScene(SceneEditor2* scene)
{
	bool enabled = scene->rulerToolSystem->IsLandscapeEditingEnabled();

	SetWidgetsState(enabled);

	BlockAllSignals(true);

	int32 width = scene->rulerToolSystem->GetLineWidth();
	ui->sliderWidgetLineWidth->SetRangeMin(defLineWidthMinValue);
	ui->sliderWidgetLineWidth->SetRangeMax(defLineWidthMaxValue);
	ui->sliderWidgetLineWidth->SetValue(width);
	UpdateLengths(activeScene,
				  activeScene->rulerToolSystem->GetLength(),
				  activeScene->rulerToolSystem->GetPreviewLength());

	dockWidget->setVisible(enabled);
	BlockAllSignals(!enabled);

	
}

void RulerToolPropertiesView::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	UpdateFromScene(scene);
}

void RulerToolPropertiesView::SceneDeactivated(SceneEditor2* scene)
{
	activeScene = NULL;
}

void RulerToolPropertiesView::Toggle()
{
	if (!activeScene)
	{
		return;
	}

	bool enabled = activeScene->rulerToolSystem->IsLandscapeEditingEnabled();
	if (!enabled)
	{
		if (activeScene->rulerToolSystem->EnableLandscapeEditing())
		{
			SetWidgetsState(true);

			activeScene->rulerToolSystem->SetLineWidth(ui->sliderWidgetLineWidth->GetValue());

			dockWidget->show();
		}
		else
		{
			QMessageBox::critical(0,
								  ResourceEditor::RULER_TOOL_ERROR_CAPTION.c_str(),
								  ResourceEditor::RULER_TOOL_ERROR_MESSAGE.c_str());
		}
	}
	else
	{
		if (activeScene->rulerToolSystem->DisableLandscapeEdititing())
		{
			SetWidgetsState(false);
			dockWidget->hide();
		}
	}

	toolbarAction->setChecked(activeScene->rulerToolSystem->IsLandscapeEditingEnabled());
}

void RulerToolPropertiesView::SetLineWidth(int width)
{
	if (!activeScene)
	{
		return;
	}

	if (width > 0)
	{
		activeScene->rulerToolSystem->SetLineWidth(width);
	}
}

void RulerToolPropertiesView::UpdateLengths(SceneEditor2 *scene, double length, double previewLength)
{
	if (scene != activeScene)
	{
		return;
	}

	if (length < 0.0)
	{
		length = 0.0;
	}
	if (previewLength < 0.0)
	{
		previewLength = 0.0;
	}

	ui->labelLength->setText(QString::number(length));
	ui->labelPreview->setText(QString::number(previewLength));
}
