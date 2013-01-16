#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

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

protected:
	DAVA::SceneNode *curNode;
};

/*
template<typename T, template <typename K, typename V> class >
class IntrospectionCollection : public DAVA::IntrospectionMember
{
	std::vector;
};
*/

#endif // __QT_PROPERTY_WIDGET_H__
