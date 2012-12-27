#include "QtPropertyBrowser/Dava/qtDavaVectorEditorFactory.h"

// QtColorEditorFactoryPrivate
void QtColorEditorFactoryPrivate::slotPropertyChanged(QtProperty *property,
	const QColor &value)
{
	const PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
	if (it == m_createdEditors.end())
		return;
	QListIterator<QtColorEditWidget *> itEditor(it.value());

	while (itEditor.hasNext())
		itEditor.next()->setValue(value);
}

void QtColorEditorFactoryPrivate::slotSetValue(const QColor &value)
{
	QObject *object = q_ptr->sender();
	const EditorToPropertyMap::ConstIterator ecend = m_editorToProperty.constEnd();
	for (EditorToPropertyMap::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != ecend; ++itEditor)
		if (itEditor.key() == object) {
			QtProperty *property = itEditor.value();
			QtColorPropertyManager *manager = q_ptr->propertyManager(property);
			if (!manager)
				return;
			manager->setValue(property, value);
			return;
		}
}

/*!
    \class QtColorEditorFactory

    \brief The QtColorEditorFactory class provides color editing  for
    properties created by QtColorPropertyManager objects.

    \sa QtAbstractEditorFactory, QtColorPropertyManager
*/

/*!
    Creates a factory with the given \a parent.
*/
QtColorEditorFactory::QtColorEditorFactory(QObject *parent) :
    QtAbstractEditorFactory<QtColorPropertyManager>(parent),
    d_ptr(new QtColorEditorFactoryPrivate())
{
    d_ptr->q_ptr = this;
}

/*!
    Destroys this factory, and all the widgets it has created.
*/
QtColorEditorFactory::~QtColorEditorFactory()
{
    qDeleteAll(d_ptr->m_editorToProperty.keys());
    delete d_ptr;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtColorEditorFactory::connectPropertyManager(QtColorPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty*,QColor)),
            this, SLOT(slotPropertyChanged(QtProperty*,QColor)));
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
QWidget *QtColorEditorFactory::createEditor(QtColorPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    QtColorEditWidget *editor = d_ptr->createEditor(property, parent);
    editor->setValue(manager->value(property));
    connect(editor, SIGNAL(valueChanged(QColor)), this, SLOT(slotSetValue(QColor)));
    connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
    return editor;
}

/*!
    \internal

    Reimplemented from the QtAbstractEditorFactory class.
*/
void QtColorEditorFactory::disconnectPropertyManager(QtColorPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty*,QColor)), this, SLOT(slotPropertyChanged(QtProperty*,QColor)));
}

