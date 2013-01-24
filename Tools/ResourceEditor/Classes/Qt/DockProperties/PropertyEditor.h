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

#if 0
template<template <typename, typename> class C, typename T, typename A>
class IntrospectionCollection : public IntrospectionCollectionBase
{
public:
	IntrospectionCollection(C<T, A>& _collection)
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

	int Size()
	{
		return collection->size();
	}

	void* Begin()
	{
		void* i = NULL;

		C<T, A>::iterator begin = collection->begin();
		C<T, A>::iterator end = collection->end();

		if(begin != end)
		{
			CollectionPos *pos = new CollectionPos();
			pos->curPos = begin;
			pos->endPos = end;

			i = (void*) pos;
		}

		return i;
	}

	void* Next(void* i)
	{
		CollectionPos* pos = (CollectionPos *) i;

		if(NULL != pos)
		{
			pos->curPos++;

			if(pos->curPos == pos->endPos)
			{
				delete pos;
				i = NULL;
			}
		}

		return i;
	}

	void Finish(void* i)
	{
		CollectionPos* pos = (CollectionPos *) i;
		if(NULL != pos)
		{
			delete pos;
		}
	}

	void GetValue(void* i, void *dst)
	{
		CollectionPos* pos = (CollectionPos *) i;
		if(NULL != pos)
		{
			T *dstT = (T*) dst;
			*dstT = *(pos->curPos);
		}
	}

	void SetValue(void* i, void *src)
	{
		CollectionPos* pos = (CollectionPos *) i;
		if(NULL != pos)
		{
			T *srcT = (T*) src;
			*(pos->curPos) = *srcT;
		}
	}

	void* Pointer(void *i)
	{
		void *p = NULL;
		CollectionPos* pos = (CollectionPos *) i;

		if(NULL != pos)
		{
			p = &(*(pos->curPos));
		}

		return p;
	}

protected:
	struct CollectionPos
	{
		typename C<T, A>::iterator curPos;
		typename C<T, A>::iterator endPos;
	};

	C<T, A> *collection;
};

template<template <typename, typename> class Container, class T, class A>
static IntrospectionCollectionBase* CreateIntrospectionCollection(Container<T, A> &t)
{
	return new IntrospectionCollection<Container, T, A>(t);
}
#endif
#endif // __QT_PROPERTY_WIDGET_H__
