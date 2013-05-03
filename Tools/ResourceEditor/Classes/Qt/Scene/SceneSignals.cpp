#include "SceneSignals.h"

void SceneSignals::EmitMouseOver(SceneEditorProxy *scene, const EntityGroup *entities)
{
	emit MouseOver(scene, entities);
}

void SceneSignals::EmitMouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities)
{
	emit MouseOverSelection(scene, entities);
}

void SceneSignals::EmitSelected(SceneEditorProxy *scene, const EntityGroup *entities)
{
	emit Selected(scene, entities);
}

void SceneSignals::EmitDeselected(SceneEditorProxy *scene, const EntityGroup *entities)
{
	emit Deselected(scene, entities);
}
