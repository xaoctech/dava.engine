#include "QtPropertyEditor/QtPropertyEditor.h"
#include "QtPropertyEditor/QtPropertyModel.h"
#include "QtPropertyEditor/QtPropertyItemDelegate.h"

QtPropertyEditor::QtPropertyEditor(QWidget *parent /* = 0 */)
	: QTreeView(parent)
{
	curModel = new QtPropertyModel();
	setModel(curModel);

	curItemDelegate = new QtPropertyItemDelegate();
	setItemDelegate(curItemDelegate);
}

QtPropertyEditor::~QtPropertyEditor()
{ }

QPair<QtPropertyItem*, QtPropertyItem*> QtPropertyEditor::AppendProperty(const QString &name, QtPropertyData* data, QtPropertyItem* parent /*= NULL*/)
{
	return curModel->AppendProperty(name, data, parent);
}

void QtPropertyEditor::RemoveProperty(QtPropertyItem* item)
{
	curModel->RemoveProperty(item);
}

void QtPropertyEditor::RemovePropertyAll()
{
	curModel->RemovePropertyAll();
}

void QtPropertyEditor::Expand(QtPropertyItem *item)
{
	expand(curModel->indexFromItem(item));
}


