#include <QtGui>

#include "QtPropertyItemDelegate.h"
#include "QtPropertyModel.h"
#include "QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

void QtPropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize QtPropertyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget* QtPropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QWidget* editWidget = NULL;
	const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());

	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
			editWidget = data->CreateEditor(parent, option);
		}

		TryEditorWorkarounds(editWidget);
	}

	if(NULL == editWidget)
	{
		editWidget = QStyledItemDelegate::createEditor(parent, option, index);
	}

    return editWidget;
}

void QtPropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
            data->SetEditorData(editor);
		}
	}

    QStyledItemDelegate::setEditorData(editor, index);
}

void QtPropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
            data->EditorDone(editor);
			//item->setIcon(data->GetIcon());
		}
	}

    QStyledItemDelegate::setModelData(editor, model, index);
}

void QtPropertyItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);

	// tune widget border and geometry
	if(NULL != editor)
	{
		editor->setObjectName("customPropertyEditor");
		editor->setStyleSheet("#customPropertyEditor{ border: 2px solid gray; }");
		QRect r = option.rect;
		r.adjust(0, -2, 0, 2);

		// check if item has icon and move editor right if it has one
        /*
		const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
		if(NULL != propertyModel)
		{
			QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
			QtPropertyData* data = item->GetPropertyData();

			if(!data->GetIcon().isNull())
			{
				r.adjust(20, 0, 0, 0);
			}
		}
        */
		editor->setGeometry(r);
	}
}

void QtPropertyItemDelegate::TryEditorWorkarounds(QWidget *editor) const
{
	if(NULL != editor)
	{
		// workaround: qtColorLineEdit should know about Object that filters it events, that object is this delegate. 
		// This fix is done for MacOS, to prevent editing cancellation, when color dialog appears
		QtColorLineEdit *lineEdit = dynamic_cast<QtColorLineEdit *>(editor);
		if(NULL != lineEdit)
		{
			lineEdit->SetItemDelegatePtr(this);
		}

		// TODO: other workarounds
		// ...
	}
}