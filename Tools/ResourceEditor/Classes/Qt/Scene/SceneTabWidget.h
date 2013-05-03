#ifndef __SCENE_TAB_WIDGET_H__
#define __SCENE_TAB_WIDGET_H__

#include <QMap>
#include <QTabBar>
#include <QWidget>
#include <QMetaType>

#include "Qt/Main/davaglwidget.h"
#include "Qt/Scene/EntityGroup.h"
#include "Qt/Scene/SceneSignals.h"
#include "Qt/Scene/SceneTypes.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"

// old ui. should be removed later -->
class SceneEditorScreenMain;
// <--
class SceneEditorProxy;
class DAVAUI3DView;

Q_DECLARE_METATYPE(SceneEditorProxy *);

class SceneTabWidget : public QWidget
{
	Q_OBJECT

public:
	SceneTabWidget(QWidget *parent);
	~SceneTabWidget();

	int OpenTab();
	int OpenTab(const DAVA::FilePath &scenePapth);
	void CloseTab(int index);

	int GetCurrentTab() const;
	void SetCurrentTab(int index);

	ST_ModifMode GetModifMode() const;
	void SetModifMode(ST_ModifMode mode);

	ST_PivotPoint GetPivotPoint() const;
	void SetPivotPoint(ST_PivotPoint pivotpoint);

	ST_Axis GetModifAxis() const;
	void SetModifAxis(ST_Axis axis);

	int GetSelectionDrawMode() const;
	void SetSelectionDrawMode(int mode);

	int GetCollisionDrawMode() const;
	void SetCollisionDrawMode(int mode);
	
public slots:
	// this slot redirects any UIEvent to the active sceneProxy for processing
	void ProcessDAVAUIEvent(DAVA::UIEvent *event);

	// tab switched by user
	void TabBarCurrentChanged(int index);

	// tab request close
	void TabBarCloseRequest(int index);

	// scene mouse over selected object
	void MouseOverSelectedEntities(SceneEditorProxy* scene, const EntityGroup *entities);

// old ui. should be removed later -->
protected:
	SceneEditorScreenMain * sceneEditorScreenMain;
	const int oldScreenID;
	bool oldInput;

	void InitOldUI();
	void ReleaseOldUI();
// <--

protected:
	QTabBar *tabBar;
	DavaGLWidget *davaWidget;
	DAVA::UIScreen *davaUIScreen;
	DAVA::UI3DView *dava3DView;
	const int davaUIScreenID;
	const int dava3DViewMargin;

	void InitDAVAUI();
	void ReleaseDAVAUI();

	SceneEditorProxy* GetTabScene(int index) const;
	void SetTabScene(int index, SceneEditorProxy* scene);

	virtual void resizeEvent(QResizeEvent * event);

private:
	int newSceneCounter;

	ST_Axis curModifAxis;
	ST_ModifMode curModifMode;
	ST_PivotPoint curPivotPoint;
	int curSelDrawMode;
	int curColDrawMode;
};

// this is helper class
// it is only used to grab all UIEvents from 3dView and
// redirect them to the specified SceneTabWidget
class DAVAUI3DView : public DAVA::UI3DView
{
public:
	DAVAUI3DView(SceneTabWidget *tw, const DAVA::Rect &rect) 
		: DAVA::UI3DView(rect)
		, tabWidget(tw)
	{
		SetInputEnabled(true);
	}

	virtual void Input(DAVA::UIEvent *event)
	{
		if(NULL != tabWidget)
		{
			tabWidget->ProcessDAVAUIEvent(event);
		}
	}

protected:
	SceneTabWidget *tabWidget;
};

#endif // __SCENE_TAB_WIDGET_H__
