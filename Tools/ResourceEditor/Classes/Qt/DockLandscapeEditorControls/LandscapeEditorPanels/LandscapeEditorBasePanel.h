#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORBASEPANEL__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORBASEPANEL__

#include <QWidget>
#include "DAVAEngine.h"

class SceneEditor2;

class LandscapeEditorBasePanel: public QWidget
{
	Q_OBJECT
	
public:
	explicit LandscapeEditorBasePanel(QWidget* parent = 0);
	virtual ~LandscapeEditorBasePanel();
	
	void InitPanel();
	void DeinitPanel();

protected slots:
	virtual void EditorToggled(SceneEditor2* scene);

private slots:
	void SceneActivated(SceneEditor2* scene);
	void SceneDeactivated(SceneEditor2* scene);

protected:
	virtual void OnSceneActivated();
	virtual void OnSceneDeactivated();

	virtual void OnEditorEnabled();
	virtual void OnEditorDisabled();

	SceneEditor2* GetActiveScene();
	virtual bool GetEditorEnabled() = 0;

	virtual void SetWidgetsState(bool enabled) = 0;
	virtual void BlockAllSignals(bool block) = 0;

	virtual void InitUI() = 0;
	virtual void ConnectToSignals() = 0;

	virtual void StoreState() = 0;
	virtual void RestoreState() = 0;

private:
	SceneEditor2* activeScene;
	
	void ConnectToBaseSignals();
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORBASEPANEL__) */
