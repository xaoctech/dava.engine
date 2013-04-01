#include "GameCore.h"
#include "Scene/SceneTabWidget.h"

#include <QVBoxLayout>

SceneTabWidget::SceneTabWidget(QWidget *parent)
	: QWidget(parent)
	, davaUIScreenID()
{
	// init all DAVA things
	InitDAVAUI();

	// create Qt controls and add them into layout
	tabBar = new QTabBar(this);
	davaWidget = new DavaGLWidget(this);
	davaWidget->setFocusPolicy(Qt::StrongFocus);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(tabBar);
	layout->addWidget(davaWidget);
	layout->setMargin(1);
	setLayout(layout);
}

SceneTabWidget::~SceneTabWidget()
{

}

void SceneTabWidget::InitDAVAUI()
{
	davaUIScreen = new DAVA::UIScreen();
}

void SceneTabWidget::setCurrentIndex(int index)
{

}