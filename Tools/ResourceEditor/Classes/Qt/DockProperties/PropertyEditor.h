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
	void Test();

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


class IntrospectionCollectionBase
{
public:
	virtual DAVA::MetaInfo* CollectionType() = 0;
	virtual DAVA::MetaInfo* ValueType() = 0;
};

template<template <typename, typename> class C, typename T, typename A>
class IntrospectionCollection : public IntrospectionCollectionBase
{
public:
	IntrospectionCollection(const C<T, A>& _collection)
		: collection(&_collection)
	{ }

	DAVA::MetaInfo* CollectionType()
	{
		return DAVA::MetaInfo::Instance<C<T, A> >();
	}

	DAVA::MetaInfo* ValueType()
	{
		return DAVA::MetaInfo::Instance<T>();
	}

protected:
	const C<T, A> *collection;
};

template<template <typename, typename> class Container, class T, class A>
IntrospectionCollectionBase* fnTest(Container<T, A> &t)
{
	return new IntrospectionCollection<Container, T, A>(t);
}

#endif // __QT_PROPERTY_WIDGET_H__
