#include "QtPropertyBrowser/Dava/qtDavaVectorEditorFactory.h"

QDoubleSpinBox* QtVector4EditorFactoryPrivate::createEditor(QtProperty *property, QWidget *parent)
{
	QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
	initializeEditor(property, editor);
	return editor;
}

void QtVector4EditorFactoryPrivate::initializeEditor(QtProperty *property, QDoubleSpinBox *editor)
{
	Q_TYPENAME PropertyToEditorListMap::iterator it = m_createdEditors.find(property);
	if (it == m_createdEditors.end())
		it = m_createdEditors.insert(property, EditorList());
	it.value().append(editor);
	m_editorToProperty.insert(editor, property);
}

void QtVector4EditorFactoryPrivate::slotEditorDestroyed(QObject *object)
{
	const Q_TYPENAME EditorToPropertyMap::iterator ecend = m_editorToProperty.end();
	for (Q_TYPENAME EditorToPropertyMap::iterator itEditor = m_editorToProperty.begin(); itEditor !=  ecend; ++itEditor) {
		if (itEditor.key() == object) {
			QDoubleSpinBox *editor = itEditor.key();
			QtProperty *property = itEditor.value();
			const Q_TYPENAME PropertyToEditorListMap::iterator pit = m_createdEditors.find(property);
			if (pit != m_createdEditors.end()) {
				pit.value().removeAll(editor);
				if (pit.value().empty())
					m_createdEditors.erase(pit);
			}
			m_editorToProperty.erase(itEditor);
			return;
		}
	}
}

void QtVector4EditorFactoryPrivate::slotPropertyChanged(QtProperty *property, double value)
{
	QList<QDoubleSpinBox *> editors = m_createdEditors[property];
	QListIterator<QDoubleSpinBox *> itEditor(m_createdEditors[property]);
	while (itEditor.hasNext()) {
		QDoubleSpinBox *editor = itEditor.next();
		if (editor->value() != value) {
			editor->blockSignals(true);
			editor->setValue(value);
			editor->blockSignals(false);
		}
	}
}

void QtVector4EditorFactoryPrivate::slotSetValue(double value)
{
	QObject *object = q_ptr->sender();
	const QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator itcend = m_editorToProperty.constEnd();
	for (QMap<QDoubleSpinBox *, QtProperty *>::ConstIterator itEditor = m_editorToProperty.constBegin(); itEditor != itcend; ++itEditor) {
		if (itEditor.key() == object) {
			QtProperty *property = itEditor.value();
			QtVector4EditorFactoryPrivate *manager = 0; //q_ptr->propertyManager(property);
			if (!manager)
				return;
			//manager->setValue(property, value);
			return;
		}
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

