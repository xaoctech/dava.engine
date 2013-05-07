#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditorProxy.h"

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditorProxy;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
	Q_OBJECT

signals:
	void Opened(SceneEditorProxy *scene);
	void Closed(SceneEditorProxy *scene);

	void Loaded(SceneEditorProxy *scene);
	void Saved(SceneEditorProxy *scene);

	void Activated(SceneEditorProxy *scene);
	void Deactivated(SceneEditorProxy *scene);

	void Selected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void Deselected(SceneEditorProxy *scene, DAVA::Entity *entity);

	void MouseOver(SceneEditorProxy *scene, const EntityGroup *entities);
	void MouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities);

public:
	void EmitOpened(SceneEditorProxy *scene) { emit Opened(scene); }
	void EmitClosed(SceneEditorProxy *scene) { emit Closed(scene); }

	void EmitLoaded(SceneEditorProxy *scene) { emit Loaded(scene); }
	void EmitSaved(SceneEditorProxy *scene) { emit Saved(scene); }

	void EmitActivated(SceneEditorProxy *scene) { emit Activated(scene); }
	void EmitDeactivated(SceneEditorProxy *scene) { emit Deactivated(scene); }

	void EmitSelected(SceneEditorProxy *scene, DAVA::Entity *entity) { emit Selected(scene, entity); }
	void EmitDeselected(SceneEditorProxy *scene, DAVA::Entity *entity)  { emit Deselected(scene, entity); }

	void EmitMouseOver(SceneEditorProxy *scene, const EntityGroup *entities) { emit MouseOver(scene, entities); }
	void EmitMouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities) { emit MouseOverSelection(scene, entities); }
};

#endif // __SCENE_MANAGER_H__
