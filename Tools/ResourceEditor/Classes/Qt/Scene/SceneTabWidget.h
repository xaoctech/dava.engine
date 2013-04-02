#ifndef __SCENE_TAB_WIDGET_H__
#define __SCENE_TAB_WIDGET_H__

#include <QTabBar>
#include "Main/davaglwidget.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"

class SceneTabWidget : public QWidget
{
	Q_OBJECT

public:
	SceneTabWidget(QWidget *parent);
	~SceneTabWidget();

public slots:
	void setCurrentIndex(int index);

protected:
	QTabBar *tabBar;

	DavaGLWidget *davaWidget;
	DAVA::UIScreen *davaUIScreen;
	DAVA::UI3DView *dava3DView;
	int davaUIScreenID;

	void InitDAVAUI();
};

#endif // __SCENE_TAB_WIDGET_H__
