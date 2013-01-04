#include <QtGui>

#include "QtPropertyItemDelegate.h"
#include "QtPropertyModel.h"

void QtPropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// TODO:
	// custom paint

    QStyledItemDelegate::paint(painter, option, index);
}

QSize QtPropertyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// TODO:
	// custom sizeHint

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
	}

	if(NULL == editWidget)
	{
		editWidget = QStyledItemDelegate::createEditor(parent, option, index);
	}

    return editWidget;
}

void QtPropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	// TODO:
	// custom setEditorData

    QStyledItemDelegate::setEditorData(editor, index);
}

void QtPropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	// TODO:
	// custom setModelData

    QStyledItemDelegate::setModelData(editor, model, index);
}

void QtPropertyItemDelegate::commitAndCloseEditor()
{
    //StarEditor *editor = qobject_cast<StarEditor *>(sender());
    //emit commitData(editor);
    //emit closeEditor(editor);
}

