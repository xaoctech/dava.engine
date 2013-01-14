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

template<typename T> 
class HasIntrospection
{
	class yes {	char m; }; 
	class no { yes m[2]; };

	struct BaseTest
	{ 
		const DAVA::IntrospectionInfo* GetTypeInfo();
	}; 

	struct Base : public T, public BaseTest 
	{}; 

	template <typename C, C c> struct Helper
	{}; 

	/*
	template<typename T>
	struct getter
	{
		template<typename U>
		static const DAVA::IntrospectionInfo* TypeInfo()
		{
			return NULL;
		}
	};

	template<>
	struct getter<yes>
	{
		template<typename U>
		static const DAVA::IntrospectionInfo* TypeInfo()
		{
			return U::GetTypeInfo();
		}
	};

	template<>
	struct getter<no>
	{
		template<typename U>
		static const DAVA::IntrospectionInfo* TypeInfo()
		{
			return NULL;
		}
	};
	*/

	template <typename U> 
	static no deduce(U*, Helper<const DAVA::IntrospectionInfo* (BaseTest::*)(), &U::GetTypeInfo>* = 0); 
	static yes deduce(...);

public: 
	static const bool result = sizeof(yes) == sizeof(deduce((Base*)(0)));

	/*
	static const DAVA::IntrospectionInfo* introspection()
	{
		getter<typename deduce((Base*)(0))::type> g;
		return g.TypeInfo<T>();
	}
	*/
};

template<bool C, typename T = void>
struct enable_if
{
	typedef T type;
};

template<typename T>
struct enable_if<false, T> 
{ };

template<typename T> 
typename enable_if<HasIntrospection<T>::result, const DAVA::IntrospectionInfo*>::type 
	doSomething(T * t) 
{
	return T::TypeInfo();
}

template<typename T> 
typename enable_if<!HasIntrospection<T>::result, const DAVA::IntrospectionInfo*>::type
	doSomething(T * t) 
{
	return NULL;
}

#endif // __QT_PROPERTY_WIDGET_H__
