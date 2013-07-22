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

#include "ModificationWidget.h"
#include "ui_ModificationWidget.h"
#include "Commands2/TransformCommand.h"

#include <QKeyEvent>

ModificationWidget::ModificationWidget(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::ModificationWidget)
	, groupMode(false)
	, modifMode(ModifyAbsolute)
{
	ui->setupUi(this);

	QObject::connect(ui->xAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinishedX()));
	QObject::connect(ui->yAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinishedY()));
	QObject::connect(ui->zAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinishedZ()));

	QObject::connect(SceneSignals::Instance(), SIGNAL(Selected(SceneEditor2 *, DAVA::Entity *)), this, SLOT(OnSceneEntitySelected(SceneEditor2 *, DAVA::Entity *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deselected(SceneEditor2 *, DAVA::Entity *)), this, SLOT(OnSceneEntityDeselected(SceneEditor2 *, DAVA::Entity *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(OnSceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(OnSceneDeactivated(SceneEditor2 *)));

}

ModificationWidget::~ModificationWidget()
{
	delete ui;
}

void ModificationWidget::SetMode(ModificationMode mode)
{
	modifMode = mode;
	ReloadValues();
}

void ModificationWidget::ReloadValues()
{
	if(NULL != curScene)
	{
		const EntityGroup* selection = curScene->selectionSystem->GetSelection();

		if(NULL != selection && selection->Size() > 0)
		{
			ui->xAxisModify->setEnabled(true);
			ui->yAxisModify->setEnabled(true);
			ui->zAxisModify->setEnabled(true);

			if(selection->Size() > 1)
			{
				groupMode = true;

				ui->xAxisModify->clear();
				ui->yAxisModify->clear();
				ui->zAxisModify->clear();
			}
			else
			{
				groupMode = false;

				if(modifMode == ModifyRelative)
				{
					ui->xAxisModify->setValue(0);
					ui->yAxisModify->setValue(0);
					ui->zAxisModify->setValue(0);
				}
				else
				{
					DAVA::Entity *singleEntity = selection->GetEntity(0);
					if(NULL != singleEntity)
					{
						DAVA::Matrix4 localMatrix = singleEntity->GetLocalTransform();
						DAVA::Vector3 translation = localMatrix.GetTranslationVector();

						ui->xAxisModify->setValue(translation.x);
						ui->yAxisModify->setValue(translation.y);
						ui->zAxisModify->setValue(translation.z);
					}
				}
			}
		}
		else
		{
			ui->xAxisModify->clear();
			ui->xAxisModify->setEnabled(false);
			ui->yAxisModify->clear();
			ui->yAxisModify->setEnabled(false);
			ui->zAxisModify->clear();
			ui->zAxisModify->setEnabled(false);
		}
	}
}

void ModificationWidget::ApplyValues(ST_Axis axis)
{
	DAVA::float32 x = (DAVA::float32) ui->xAxisModify->value();
	DAVA::float32 y = (DAVA::float32) ui->yAxisModify->value();
	DAVA::float32 z = (DAVA::float32) ui->zAxisModify->value();

	if(NULL != curScene)
	{
		const EntityGroup* selection = curScene->selectionSystem->GetSelection();
		if(NULL != selection)
		{
			for (size_t i = 0; i < selection->Size(); ++i)
			{
				DAVA::Entity *entity = selection->GetEntity(i);
				if(NULL != entity)
				{
					DAVA::Matrix4 origMatrix = entity->GetLocalTransform();
					DAVA::Vector3 origPos = origMatrix.GetTranslationVector();
					DAVA::Vector3 newPos = origPos;

					if(modifMode == ModifyAbsolute)
					{
						switch (axis)
						{
						case ST_AXIS_X:
							newPos.x = x;
							break;
						case ST_AXIS_Y:
							newPos.y = y;
							break;
						case ST_AXIS_Z:
							newPos.z = z;
							break;
						default:
							break;
						}
					}
					else
					{
						switch (axis)
						{
						case ST_AXIS_X:
							newPos.x += x;
							break;
						case ST_AXIS_Y:
							newPos.y += y;
							break;
						case ST_AXIS_Z:
							newPos.z += z;
							break;
						default:
							break;
						}
					}

					if(newPos != origPos)
					{
						DAVA::Matrix4 newMatrix = origMatrix;
						newMatrix.SetTranslationVector(newPos);

						curScene->Exec(new TransformCommand(entity,	origMatrix, newMatrix));
					}
				}
			}
		}
	}

	ReloadValues();
}

void ModificationWidget::OnEditingFinishedX()
{
	ApplyValues(ST_AXIS_X);
}

void ModificationWidget::OnEditingFinishedY()
{
	ApplyValues(ST_AXIS_Y);
}

void ModificationWidget::OnEditingFinishedZ()
{
	ApplyValues(ST_AXIS_Z);
}

void ModificationWidget::OnSceneActivated(SceneEditor2 *scene)
{
	curScene = scene;
	ReloadValues();
}

void ModificationWidget::OnSceneDeactivated(SceneEditor2 *scene)
{
	curScene = NULL;
	ReloadValues();
}

void ModificationWidget::OnSceneEntitySelected(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(curScene == scene)
	{
		ReloadValues();
	}
}

void ModificationWidget::OnSceneEntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(curScene == scene)
	{
		ReloadValues();
	}
}
