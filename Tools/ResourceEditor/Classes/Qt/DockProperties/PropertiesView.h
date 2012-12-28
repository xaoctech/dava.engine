#ifndef __DOCK_PROPERTIES_VIEW_H__
#define __DOCK_PROPERTIES_VIEW_H__

#include "DAVAEngine.h"
#include "Base/Introspection.h"
#include "Scene/SceneData.h"
#include "QtPropertyBrowser/qttreepropertybrowser.h"
#include "QtPropertyBrowser/qtgroupboxpropertybrowser.h"
#include "QtPropertyBrowser/qtpropertymanager.h"
#include "QtPropertyBrowser/qteditorfactory.h"
#include "QtPropertyBrowser/Dava/qtDavaVectorPropertyManager.h"

#include <QMap>
#include <QVector>
#include <QSize>

class PropertiesView : public QtTreePropertyBrowser // QtGroupBoxPropertyBrowser
{
	Q_OBJECT

public:
	PropertiesView(QWidget *parent = 0);
	~PropertiesView();

	void SetNode(DAVA::SceneNode *node);

public slots:
	void sceneActivated(SceneData *scene);
	void sceneChanged(SceneData *scene);
	void sceneReleased(SceneData *scene);
	void sceneNodeSelected(SceneData *scene, DAVA::SceneNode *node);

protected slots:
	void PropertyChangedInt(QtProperty *property);
	void PropertyChangedBool(QtProperty *property);
	void PropertyChangedEnum(QtProperty *property);
	void PropertyChangedString(QtProperty *property);
	void PropertyChangedDouble(QtProperty *property);
	void PropertyChangedVector4(QtProperty *property);
	void PropertyChangedVector3(QtProperty *property);

protected:
	DAVA::SceneNode* curNode;
	QWidget *oneForAllParent;

	QtGroupPropertyManager *managerGroup;
	QtIntPropertyManager *managerInt;
	QtBoolPropertyManager *managerBool;
	QtEnumPropertyManager *managerEnum;
	QtStringPropertyManager *managerString;
	QtDoublePropertyManager *managerDouble;
	QtVector2PropertyManager *managerVector2;
	QtVector3PropertyManager *managerVector3;
	QtVector4PropertyManager *managerVector4;

	QtSpinBoxFactory *editorInt;
	QtCheckBoxFactory *editorBool;
	QtLineEditFactory *editorString;
	QtEnumEditorFactory *editorEnum;
	QtDoubleSpinBoxFactory *editorDouble;

	QMap<QtProperty *, const DAVA::IntrospectionMember *> allProperties;

	void ClearAllProperties();
	QtProperty *CreateProperty(const DAVA::IntrospectionMember *member);

	template <typename T>
	void PropertyChange(QtProperty *property, T value);
};

#endif // __DOCK_PROPERTIES_VIEW_H__
