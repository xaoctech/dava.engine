#include "SceneManager.h"

void SceneManager::EmitMouseOver(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit MouseOver(scene, entity);
}

void SceneManager::EmitMouseOverSelection(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit MouseOverSelection(scene, entity);
}

void SceneManager::EmitSelected(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit Selected(scene, entity);
}

void SceneManager::EmitDeselected(SceneEditorProxy *scene, DAVA::Entity *entity)
{
	emit Deselected(scene, entity);
}
