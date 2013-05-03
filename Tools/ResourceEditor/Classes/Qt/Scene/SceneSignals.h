#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "Scene/EntityGroup.h"

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditorProxy;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
	Q_OBJECT

signals:
	void MouseOver(SceneEditorProxy *scene, const EntityGroup *entities);
	void MouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities);

	void Selected(SceneEditorProxy *scene, const EntityGroup *entities);
	void Deselected(SceneEditorProxy *scene, const EntityGroup *entities);

public:
	void EmitMouseOver(SceneEditorProxy *scene, const EntityGroup *entities);
	void EmitMouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities);

	void EmitSelected(SceneEditorProxy *scene, const EntityGroup *entities);
	void EmitDeselected(SceneEditorProxy *scene, const EntityGroup *entities);
};

#endif // __SCENE_MANAGER_H__
