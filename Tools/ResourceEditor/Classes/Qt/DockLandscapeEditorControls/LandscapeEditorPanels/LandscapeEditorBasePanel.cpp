#include "LandscapeEditorBasePanel.h"

#include "../../Scene/SceneSignals.h"
#include "../../Scene/SceneEditor2.h"

LandscapeEditorBasePanel::LandscapeEditorBasePanel(QWidget* parent)
:	QWidget(parent)
,	activeScene(NULL)
{
	ConnectToBaseSignals();
}

LandscapeEditorBasePanel::~LandscapeEditorBasePanel()
{
}

void LandscapeEditorBasePanel::ConnectToBaseSignals()
{
	connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2*)), this, SLOT(SceneActivated(SceneEditor2*)));
	connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2*)), this, SLOT(SceneDeactivated(SceneEditor2*)));
}

void LandscapeEditorBasePanel::SceneActivated(SceneEditor2* scene)
{
	DVASSERT(scene);
	activeScene = scene;
	RestoreState();
	OnSceneActivated();
}

void LandscapeEditorBasePanel::SceneDeactivated(SceneEditor2* scene)
{
	if (activeScene)
	{
		StoreState();
		OnSceneDeactivated();
	}
	
	activeScene = NULL;
}

SceneEditor2* LandscapeEditorBasePanel::GetActiveScene()
{
	return activeScene;
}

void LandscapeEditorBasePanel::OnSceneActivated()
{
}

void LandscapeEditorBasePanel::OnSceneDeactivated()
{
}

void LandscapeEditorBasePanel::OnEditorEnabled()
{
}

void LandscapeEditorBasePanel::OnEditorDisabled()
{
}

void LandscapeEditorBasePanel::InitPanel()
{
	bool enabled = GetEditorEnabled();
	SetWidgetsState(enabled);
	BlockAllSignals(!enabled);
	ConnectToShortcuts();
}

void LandscapeEditorBasePanel::DeinitPanel()
{
	SetWidgetsState(false);
	BlockAllSignals(true);
	DisconnectFromShortcuts();
}

void LandscapeEditorBasePanel::EditorToggled(SceneEditor2 *scene)
{
	if (scene != GetActiveScene())
	{
		return;
	}

	if (GetEditorEnabled())
	{
		RestoreState();
		OnEditorEnabled();
	}
	else
	{
		StoreState();
		OnEditorDisabled();
	}
}

void LandscapeEditorBasePanel::ConnectToShortcuts()
{
}

void LandscapeEditorBasePanel::DisconnectFromShortcuts()
{
}
