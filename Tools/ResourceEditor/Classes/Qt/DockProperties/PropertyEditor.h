#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

#include "PropertyEditorStateHelper.h"
#include "QtPosSaver/QtPosSaver.h"
#include "QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneData.h"
#include "Scene/SceneSignals.h"

class DAVA::Entity;

class PropertyEditor : public QtPropertyEditor
{
	Q_OBJECT

public:
	PropertyEditor(QWidget *parent = 0);
	~PropertyEditor();

	void SetNode(DAVA::Entity *node);
	void SetAdvancedMode(bool set);

protected:
    QtPropertyData* AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo * info);
    
public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);
	void sceneNodeSelected(SceneData *scene, DAVA::Entity *node);

	void actionShowAdvanced();

	void EntitySelected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void EntityDeselected(SceneEditorProxy *scene, DAVA::Entity *entity);

protected:
	bool advancedMode;
	QtPosSaver posSaver;

	DAVA::Entity *curNode;
	PropertyEditorStateHelper treeStateHelper;
};

#endif // __QT_PROPERTY_WIDGET_H__
