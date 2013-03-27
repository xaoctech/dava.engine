#include <QtGui>

#include "QtPropertyItemDelegate.h"
#include "QtPropertyModel.h"
#include "QtPropertyItem.h"
#include "QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

QtPropertyItemDelegate::QtPropertyItemDelegate(QWidget *parent /* = 0 */)
	: QStyledItemDelegate(parent)
{ }

void QtPropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	recalcOptionalWidgets(index, (QStyleOptionViewItem *) &option);
	QStyledItemDelegate::paint(painter, option, index);
}

QSize QtPropertyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget* QtPropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QWidget* editWidget = NULL;
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();
	
	recalcOptionalWidgets(index, (QStyleOptionViewItem *) &option);

	if(NULL != data)
	{
		editWidget = data->CreateEditor(parent, option);
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
		editor->setGeometry(r);
	}
}

void QtPropertyItemDelegate::recalcOptionalWidgets(const QModelIndex &index, QStyleOptionViewItem *option) const
{
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();

	if(NULL != data)
	{
		QWidget *owViewport = data->GetOWViewport();

		for (int i = 0; i < data->GetOWCount(); i++)
		{
			int prevOWSpace = 0;
			int owSpacing = 1;

			const QtPropertyOW *ow = data->GetOW(i);
			if(NULL != ow && NULL != owViewport && NULL != ow->widget)
			{
				QWidget *owWidget = ow->widget;
				int owWidth = ow->size.width();

				if(0 != owWidth)
				{
					QRect owRect = option->rect;
					owRect.setLeft(owRect.right() - owWidth - prevOWSpace);

					owWidget->setGeometry(owRect);
					owWidget->show();

					// if this widget isn't overlayed we should modify rect for tree view cell to be drawn in.
					if(!ow->overlay)
					{
						option->rect.setRight(owRect.left());
					}

					prevOWSpace += (owWidth + owSpacing);
				}
				else
				{
					owWidget->hide();
				}
			}
		}
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
