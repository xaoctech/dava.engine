#ifndef QTDAVAVECTOREDITORFACTORY_H
#define QTDAVAVECTOREDITORFACTORY_H

#include <QDoubleSpinBox>
#include "QtPropertyBrowser/qtpropertymanager.h"
#include "QtPropertyBrowser/Dava/qtDavaVectorPropertyManager.h"

class QtVector4EditorFactory;

class QtVector4EditorFactoryPrivate : public QtVector4PropertyManager
{
public:
	Q_DECLARE_PUBLIC(QtVector4EditorFactory)

	typedef QList<QDoubleSpinBox *> EditorList;
	typedef QMap<QtProperty *, EditorList> PropertyToEditorListMap;
	typedef QMap<QDoubleSpinBox *, QtProperty *> EditorToPropertyMap;

	QDoubleSpinBox *createEditor(QtProperty *property, QWidget *parent);
	void initializeEditor(QtProperty *property, QDoubleSpinBox *e);
	void slotEditorDestroyed(QObject *object);

	PropertyToEditorListMap  m_createdEditors;
	EditorToPropertyMap m_editorToProperty;
	QtVector4EditorFactory *q_ptr;

	void slotPropertyChanged(QtProperty *property, double value);
	void slotSetValue(double value);
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector4EditorFactory : public QtAbstractEditorFactory<QtVector4PropertyManager>
{
	Q_OBJECT
public:
	QtVector4EditorFactory(QObject *parent = 0);
	~QtVector4EditorFactory();
protected:
	void connectPropertyManager(QtVector4PropertyManager *manager);
	void disconnectPropertyManager(QtVector4PropertyManager *manager);

	QWidget *createEditor(QtVector4PropertyManager *manager, QtProperty *property, QWidget *parent);
private:
	QtVector4EditorFactoryPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector4EditorFactory)
	Q_DISABLE_COPY(QtVector4EditorFactory)
	Q_PRIVATE_SLOT(d_func(), void slotPropertyChanged(QtProperty *, const QColor &))
	Q_PRIVATE_SLOT(d_func(), void slotEditorDestroyed(QObject *))
	Q_PRIVATE_SLOT(d_func(), void slotSetValue(const QColor &))
};

#endif 
