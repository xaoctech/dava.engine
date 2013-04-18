#include "SceneEditor/SceneEditorScreenMain.h"

#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditorProxy.h"
#include "AppScreens.h"

#include <QVBoxLayout>
#include <QResizeEvent>

SceneTabWidget::SceneTabWidget(QWidget *parent)
	: QWidget(parent)
	, davaUIScreenID(SCREEN_MAIN)
	, oldScreenID(SCREEN_MAIN + 1)
	, dava3DViewMargin(10)
	, currentTabIndex(-1)
	, currentTabID(-1)
	, tabIDCounter(0)
{
	this->setMouseTracking(true);

	// create Qt controls and add them into layout
	// 
	// tab bar
	tabBar = new QTabBar(this);

	// davawidget to displae davaengine content
	davaWidget = new DavaGLWidget(this);
	davaWidget->setFocusPolicy(Qt::StrongFocus);

	// put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(tabBar);
	layout->addWidget(davaWidget);
	layout->setMargin(1);
	setLayout(layout);

	// create DAVA UI
	InitDAVAUI();
	InitOldUI();

	// remove -->
	int oldTabIndex = tabBar->addTab("OldScreenMain");
	tabBar->setTabData(oldTabIndex, -1);
	// <--

	AddTab("/Projects/dava.wot.art/DataSource/3d/Maps/dike_village/dike_village.sc2");
	//AddTab("/Projects/dava.wot.art/DataSource/3d/Maps/desert_train/desert_train.sc2");

	QObject::connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(SetCurrentTab(int)));
	SetCurrentTab(oldTabIndex);
}

SceneTabWidget::~SceneTabWidget()
{
	ReleaseOldUI();
	ReleaseDAVAUI();
}

void SceneTabWidget::InitOldUI()
{
	// old screen 
	sceneEditorScreenMain = new SceneEditorScreenMain();
	UIScreenManager::Instance()->RegisterScreen(oldScreenID, sceneEditorScreenMain);
}

void SceneTabWidget::ReleaseOldUI()
{
	SafeRelease(sceneEditorScreenMain);
}

void SceneTabWidget::InitDAVAUI()
{
	dava3DView = new DAVAUI3DView(this, DAVA::Rect(dava3DViewMargin, dava3DViewMargin, 0, 0));
	dava3DView->SetDebugDraw(true);

	davaUIScreen = new DAVA::UIScreen();
	davaUIScreen->AddControl(dava3DView);
	davaUIScreen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
	davaUIScreen->GetBackground()->SetColor(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f));

	UIScreenManager::Instance()->RegisterScreen(davaUIScreenID, davaUIScreen);
}

void SceneTabWidget::ReleaseDAVAUI()
{
	SafeRelease(davaUIScreen);
}

int SceneTabWidget::AddTab()
{
	int tabID = tabIDCounter;
	tabIDCounter++;

	SceneEditorProxy *sceneProxy = new SceneEditorProxy();
	tabIDtoSceneMap.insert(tabID, sceneProxy);

	int tabIndex = tabBar->addTab("NewScene");
	tabBar->setTabData(tabIndex, tabID);

	return tabIndex;
}

int SceneTabWidget::AddTab(const DAVA::String &scenePapth)
{
	int tabIndex = AddTab();
	int tabID = GetTabID(tabIndex);

	tabIDtoSceneMap[tabID]->Open(scenePapth);
	tabBar->setTabText(tabIndex, scenePapth.c_str());
	tabBar->setTabToolTip(tabIndex, scenePapth.c_str());

	return tabIndex;
}

int SceneTabWidget::GetCurrentTab()
{
	return currentTabIndex;
}

void SceneTabWidget::SetCurrentTab(int index)
{
	if(index != currentTabIndex && index >= 0 && index < tabBar->count())
	{
		tabBar->setCurrentIndex(index);
		
		currentTabIndex = index;
		currentTabID = GetTabID(index);

		if(currentTabID >= 0)
		{
			// old. remove -->
			oldInput = false;
			UIScreenManager::Instance()->SetScreen(davaUIScreenID);
			// <--

			SceneEditorProxy* curSceneProxy = tabIDtoSceneMap[currentTabID];
			dava3DView->SetScene(curSceneProxy);
			curSceneProxy->SetViewportRect(dava3DView->GetRect());
		}
		// old. remove -->
		else
		{
			oldInput = true;
			UIScreenManager::Instance()->SetScreen(oldScreenID);
		}
		// <--
	}
}

int SceneTabWidget::GetTabIndex(int tabID)
{
	int index = -1;
	for(int i = 0; i < tabBar->count(); ++i)
	{
		if(tabBar->tabData(i).toInt() == tabID)
		{
			index = i;
			break;
		}
	}

	return index;
}

int SceneTabWidget::GetTabID(int index)
{
	int tabID = -1;

	if(index >= 0 && index < tabBar->count())
	{
		tabID = tabBar->tabData(index).toInt();
	}

	return tabID;
}

void SceneTabWidget::ProcessDAVAUIEvent(DAVA::UIEvent *event)
{
	if(!oldInput)
	{
		SceneEditorProxy* curSceneProxy = tabIDtoSceneMap[currentTabID];
		if(NULL != curSceneProxy)
		{
			curSceneProxy->ProcessUIEvent(event);
		}
	}
}

void SceneTabWidget::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);

	if(NULL != event)
	{
		QSize s = davaWidget->size();

		davaUIScreen->SetSize(DAVA::Vector2(s.width(), s.height()));
		dava3DView->SetSize(DAVA::Vector2(s.width() - 2 * dava3DViewMargin, s.height() - 2 * dava3DViewMargin));
		sceneEditorScreenMain->SetSize(DAVA::Vector2(s.width(), s.height()));

		SceneEditorProxy* curSceneProxy = tabIDtoSceneMap[currentTabID];
		if(NULL != curSceneProxy)
		{
			curSceneProxy->SetViewportRect(dava3DView->GetRect());
		}
	}
}
