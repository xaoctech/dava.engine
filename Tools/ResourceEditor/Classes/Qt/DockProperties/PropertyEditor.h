#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

#include "PropertyEditorStateHelper.h"
#include "QtPosSaver/QtPosSaver.h"
#include "QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneData.h"

class DAVA::SceneNode;

class PropertyEditor : public QtPropertyEditor
{
	Q_OBJECT

public:
	PropertyEditor(QWidget *parent = 0);
	~PropertyEditor();

	void SetNode(DAVA::SceneNode *node);

protected:
    void AppendIntrospectionInfo(void *object, const DAVA::IntrospectionInfo * info);
    
public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);
	void sceneNodeSelected(SceneData *scene, DAVA::SceneNode *node);

	void actionHideReadOnly();

protected:
	bool hideReadOnly;
	QtPosSaver posSaver;

	DAVA::SceneNode *curNode;
	PropertyEditorStateHelper treeStateHelper;
};

#endif // __QT_PROPERTY_WIDGET_H__
