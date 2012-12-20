#ifndef __DOCK_PROPERTIES_VIEW_H__
#define __DOCK_PROPERTIES_VIEW_H__

#include "DAVAEngine.h"
#include "Base/Introspection.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"
#include "QtPropertyBrowser/qtgroupboxpropertybrowser.h"

#include <QMap>
#include <QSize>

//class QtGroupPropertyManager;
//class QtIntPropertyManager;
//class QtBoolPropertyManager;
//class QtEnumPropertyManager;
//class QtStringPropertyManager;

//class QtSpinBoxFactory;
//class QtCheckBoxFactory;
//class QtLineEditFactory;
//class QtEnumEditorFactory;

//REGISTER_META_TYPE(Matrix4);

class test1
{
public:
	int v1;
	DAVA::Matrix4 m1;
	int p1;
	int pp1;

	int getp1()
	{
		return p1;
	}

	void setp1(const int &_p1)
	{
		p1 = _p1;
		pp1 = _p1 / 2;
	}

	INTROSPECTION(test1,
		MEMBER(v1, "int test")
		MEMBER(m1, "matrix4 text")
		PROPERTY(p1, "prop1", getp1, setp1)
		);

};

//REGISTER_META_TYPE(Matrix3);
class test2 : public test1
{
public:
	DAVA::Matrix3 m1;

	INTROSPECTION_EXTEND(test2, test1,
		MEMBER(m1, "matrix3 test")
		);
};

class PropertiesView : public QtGroupBoxPropertyBrowser//QtTreePropertyBrowser
{
	Q_OBJECT

public:
	PropertiesView(QWidget *parent = 0);
	~PropertiesView();

	void SetNode(DAVA::SceneNode *node);

protected:
	DAVA::SceneNode* curNode;
};

#endif // __DOCK_PROPERTIES_VIEW_H__
