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

#include "SceneEditor/SceneEditorScreenMain.h"

#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"
#include "AppScreens.h"

#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMessageBox>

SceneTabWidget::SceneTabWidget(QWidget *parent)
	: QWidget(parent)
	, davaUIScreenID(SCREEN_MAIN)
	, dava3DViewMargin(3)
	, newSceneCounter(0)
	, curScene(NULL)
	/*
	, curModifAxis(ST_AXIS_X)
	, curModifMode(ST_MODIF_MOVE)
	, curPivotPoint(ST_PIVOT_COMMON_CENTER)
	, curSelDrawMode(ST_SELDRAW_DRAW_CORNERS | ST_SELDRAW_FILL_SHAPE)
	, curColDrawMode(ST_COLL_DRAW_NOTHING)
	*/
{
	this->setMouseTracking(true);

	// create Qt controls and add them into layout
	// 
	// tab bar
	tabBar = new QTabBar(this);
	tabBar->setTabsClosable(true);
	tabBar->setMovable(true);
	tabBar->setUsesScrollButtons(true);
	tabBar->setExpanding(false);

	// davawidget to display DAVAEngine content
	davaWidget = new DavaGLWidget(this);
	davaWidget->setFocusPolicy(Qt::StrongFocus);

	// put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(tabBar);
	layout->addWidget(davaWidget);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);

	// create DAVA UI
	InitDAVAUI();

	QObject::connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(TabBarCurrentChanged(int)));
	QObject::connect(tabBar, SIGNAL(tabCloseRequested(int)), this, SLOT(TabBarCloseRequest(int)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(MouseOverSelection(SceneEditor2*, const EntityGroup*)), this, SLOT(MouseOverSelectedEntities(SceneEditor2*, const EntityGroup*)));
}

SceneTabWidget::~SceneTabWidget()
{
	ReleaseDAVAUI();
}

void SceneTabWidget::InitDAVAUI()
{
	dava3DView = new DAVAUI3DView(this, DAVA::Rect(dava3DViewMargin, dava3DViewMargin, 0, 0));
	//dava3DView->SetDebugDraw(true);

	davaUIScreen = new DAVA::UIScreen();
	davaUIScreen->AddControl(dava3DView);
	davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));

	UIScreenManager::Instance()->RegisterScreen(davaUIScreenID, davaUIScreen);
	UIScreenManager::Instance()->SetScreen(davaUIScreenID);
}

void SceneTabWidget::ReleaseDAVAUI()
{
	SafeRelease(davaUIScreen);
}

int SceneTabWidget::OpenTab()
{
	SceneEditor2 *scene = new SceneEditor2();

	int tabIndex = tabBar->addTab("NewScene" + QString::number(++newSceneCounter));
	SetTabScene(tabIndex, scene);

	SetCurrentTab(tabIndex);
	return tabIndex;
}

int SceneTabWidget::OpenTab(const DAVA::FilePath &scenePapth)
{
	int tabIndex = -1;
	SceneEditor2 *scene = new SceneEditor2();

	if(scene->Load(scenePapth))
	{
		tabIndex = tabBar->addTab(scenePapth.GetFilename().c_str());
		SetTabScene(tabIndex, scene);

		/*
		scene->modifSystem->SetModifMode(curModifMode);
		scene->modifSystem->SetModifAxis(curModifAxis);
		scene->selectionSystem->SetPivotPoint(curPivotPoint);
		scene->collisionSystem->SetDrawMode(curColDrawMode);
		scene->selectionSystem->SetDrawMode(curSelDrawMode);
		*/

		tabBar->setTabToolTip(tabIndex, scenePapth.GetAbsolutePathname().c_str());
	}
	else
	{
		// TODO:
		// message box about can't load scene
		// ...
		
		delete scene;
	}

	SetCurrentTab(tabIndex);
	return tabIndex;
}

void SceneTabWidget::CloseTab(int index)
{
	bool doCloseScene = false;
	SceneEditor2 *scene = GetTabScene(index);

	if(NULL != scene)
	{
		if(scene->IsChanged())
		{
			int answer = QMessageBox::question(NULL, "Scene was changed", "Do you want to save changes, made to scene?", 
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);
			
			if(answer == QMessageBox::Yes)
			{
				if(scene->Save())
				{
					doCloseScene = true;
				}
				else
				{
					QMessageBox::critical(NULL, "Scene save error", "Error saving scene. Please try again.");
				}
			}
			else if(answer == QMessageBox::No)
			{
				doCloseScene = true;
			}
		}
		else
		{
			doCloseScene = true;
		}
	}

	if(doCloseScene)
	{
		if(index == tabBar->currentIndex())
		{
			dava3DView->SetScene(NULL);
			SceneSignals::Instance()->EmitDeactivated(scene);
		}

		tabBar->removeTab(index);
		delete scene;
	}
}

int SceneTabWidget::GetCurrentTab() const
{
	return tabBar->currentIndex();
}

void SceneTabWidget::SetCurrentTab(int index)
{
	if(index >= 0 && index < tabBar->count())
	{
		SceneEditor2 *oldScene = curScene;
		curScene = GetTabScene(index);

		if(NULL != oldScene)
		{
			SceneSignals::Instance()->EmitDeactivated(oldScene);
		}

		tabBar->setCurrentIndex(index);

		if(NULL != curScene)
		{
			dava3DView->SetScene(curScene);
			curScene->SetViewportRect(dava3DView->GetRect());

			SceneSignals::Instance()->EmitActivated(curScene);
		}
	}
}

SceneEditor2* SceneTabWidget::GetTabScene(int index) const
{
	SceneEditor2 *ret = NULL;

	if(index >= 0 && index < tabBar->count())
	{
		ret = tabBar->tabData(index).value<SceneEditor2 *>();
	}

	return ret;
}

void SceneTabWidget::SetTabScene(int index, SceneEditor2* scene)
{
	if(index >= 0 && index < tabBar->count())
	{
		tabBar->setTabData(index, qVariantFromValue(scene));
	}
}

void SceneTabWidget::ProcessDAVAUIEvent(DAVA::UIEvent *event)
{
	SceneEditor2* scene = GetTabScene(tabBar->currentIndex());
	if(NULL != scene)
	{
		scene->PostUIEvent(event);
	}
}

void SceneTabWidget::TabBarCurrentChanged(int index)
{
	SetCurrentTab(index);
}

void SceneTabWidget::TabBarCloseRequest(int index)
{
	CloseTab(index);
}

void SceneTabWidget::MouseOverSelectedEntities(SceneEditor2* scene, const EntityGroup *entities)
{
	/*
	if(NULL != entities)
	{
		switch (curModifMode)
		{
		case ST_MODIF_MOVE:
			setCursor(Qt::SizeAllCursor);
			break;
		case ST_MODIF_ROTATE:
			break;
		case ST_MODIF_SCALE:
			break;
		case ST_MODIF_OFF:
		default:
			setCursor(Qt::ArrowCursor);
			break;
		}
		
	}
	else
	{
		setCursor(Qt::ArrowCursor);
	}

	*/
}

void SceneTabWidget::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);

	if(NULL != event)
	{
		QSize s = davaWidget->size();

		davaUIScreen->SetSize(DAVA::Vector2(s.width(), s.height()));
		dava3DView->SetSize(DAVA::Vector2(s.width() - 2 * dava3DViewMargin, s.height() - 2 * dava3DViewMargin));

		SceneEditor2* scene = GetTabScene(tabBar->currentIndex());
		if(NULL != scene)
		{
			scene->SetViewportRect(dava3DView->GetRect());
		}
	}
}

/*
ST_ModifMode SceneTabWidget::GetModifMode() const
{
	return curModifMode;
}

void SceneTabWidget::SetModifMode(ST_ModifMode mode)
{
	if(curModifMode != mode)
	{
		curModifMode = mode;

		for(int i = 0; i < tabBar->count(); ++i)
		{
			SceneEditor2 *scene = GetTabScene(i);
			if(NULL != scene)
			{
				scene->modifSystem->SetModifMode(curModifMode);
			}
		}
	}
}

ST_PivotPoint SceneTabWidget::GetPivotPoint() const
{
	return curPivotPoint;
}

void SceneTabWidget::SetPivotPoint(ST_PivotPoint pivotpoint)
{
	if(curPivotPoint != pivotpoint)
	{
		curPivotPoint = pivotpoint;

		for(int i = 0; i < tabBar->count(); ++i)
		{
			SceneEditor2 *scene = GetTabScene(i);
			if(NULL != scene)
			{
				scene->selectionSystem->SetPivotPoint(curPivotPoint);
			}
		}
	}
}

ST_Axis SceneTabWidget::GetModifAxis() const
{
	return curModifAxis;
}

void SceneTabWidget::SetModifAxis(ST_Axis axis)
{
	if(curModifAxis != axis)
	{
		curModifAxis = axis;

		for(int i = 0; i < tabBar->count(); ++i)
		{
			SceneEditor2 *scene = GetTabScene(i);
			if(NULL != scene)
			{
				scene->modifSystem->SetModifAxis(curModifAxis);
			}
		}
	}
}

int SceneTabWidget::GetSelectionDrawMode() const
{
	return curSelDrawMode;
}

void SceneTabWidget::SetSelectionDrawMode(int mode)
{
	if(curSelDrawMode != mode)
	{
		curSelDrawMode = mode;

		for(int i = 0; i < tabBar->count(); ++i)
		{
			SceneEditor2 *scene = GetTabScene(i);
			if(NULL != scene)
			{
				scene->selectionSystem->SetDrawMode(curSelDrawMode);
			}
		}
	}
}

int SceneTabWidget::GetCollisionDrawMode() const
{
	return curColDrawMode;
}

void SceneTabWidget::SetCollisionDrawMode(int mode)
{
	if(curColDrawMode != mode)
	{
		curColDrawMode = mode;

		for(int i = 0; i < tabBar->count(); ++i)
		{
			SceneEditor2 *scene = GetTabScene(i);
			if(NULL != scene)
			{
				scene->collisionSystem->SetDrawMode(curColDrawMode);
			}
		}
	}
}
*/

SceneEditor2* SceneTabWidget::GetCurrentScene() const
{
	return GetTabScene(GetCurrentTab());
}
