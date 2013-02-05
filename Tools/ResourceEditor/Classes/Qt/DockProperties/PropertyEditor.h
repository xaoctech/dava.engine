#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

#include "QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneData.h"

class DAVA::SceneNode;

class TestBase
{
public:
	int in;

	INTROSPECTION(TestBase,
		MEMBER(in, "Test in", 0)
		);
};

class TestBaseChild : public TestBase
{
public:
	int ccc;

	INTROSPECTION_EXTEND(TestBaseChild, TestBase, 
		MEMBER(ccc, "ccc", 0)
		);
};

class TestIntro
{
public:
	TestIntro()
	{
		child1 = new TestBaseChild();
		child1->in = 111;
		((TestBaseChild *) child1)->ccc = 222;

		const DAVA::IntrospectionInfo* i1 = DAVA::GetIntrospection(child1);
		const DAVA::IntrospectionInfo* i2 = DAVA::GetIntrospectionByObject<TestBase>(child1);
		const DAVA::IntrospectionInfo* i3 = DAVA::MetaInfo::Instance<TestBase>()->GetIntrospection();
		const DAVA::IntrospectionInfo* i4 = DAVA::MetaInfo::Instance<TestBase>()->GetIntrospection(child1);

		for(int i = 0; i < 10; ++i)
		{
			TestBaseChild *tt = NULL;

			if(i & 0x1)
			{
				tt = new TestBaseChild();
				tt->ccc = i * 5;
				tt->in = i * 10;
			}

			v.push_back(tt);
		}
	}

	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
	TestBase *child1;
	DAVA::Vector<TestBase *> v;

	INTROSPECTION(TestIntro,
		MEMBER(e, "Test e", 0)
		MEMBER(f, "Test f", 0)
		MEMBER(a, "Test a", 0)
		MEMBER(b, "Test b", 0)
		MEMBER(c, "Test c", 0)
		MEMBER(d, "Test d", 0)
		MEMBER(child1, "child1", 0)
		COLLECTION(v, "Test collection v", 0)
		);
};

class PropertyEditor : public QtPropertyEditor
{
	Q_OBJECT

public:
	PropertyEditor(QWidget *parent = 0);
	~PropertyEditor();

	void SetNode(DAVA::SceneNode *node);
	void Test(DAVA::BaseObject *object);


	void SaveIntospection(void *object, const DAVA::IntrospectionInfo *info);

	DAVA::KeyedArchive* SerializeIntrospection(void *object, const DAVA::IntrospectionInfo *info);
	DAVA::KeyedArchive* SerializeCollection(void *object, const DAVA::IntrospectionCollectionBase *collection);

	void SearchIntrospection(void *object, const DAVA::IntrospectionInfo *info, DAVA::Map<void *, const DAVA::IntrospectionInfo *> *result);
	void DumpKeyedArchive(DAVA::KeyedArchive *archive, int level = 0);

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

#endif // __QT_PROPERTY_WIDGET_H__
