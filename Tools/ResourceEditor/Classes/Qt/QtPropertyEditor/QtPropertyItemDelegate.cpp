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
	recalcOptionalWidget(index, (QStyleOptionViewItem *) &option);
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
	
	recalcOptionalWidget(index, (QStyleOptionViewItem *) &option);

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

void QtPropertyItemDelegate::recalcOptionalWidget(const QModelIndex &index, QStyleOptionViewItem *option) const
{
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();

	if(NULL != data)
	{
		QWidget *optionalWidget = data->GetOptionalWidget();
		QWidget *optionalWidgetViewport = data->GetOptionalWidgetViewport();

		if(NULL != optionalWidget && NULL != optionalWidgetViewport)
		{
			int widgetSize = 0;
			QRect newWidgetRect = option->rect;

			// TODO: take widget size from QtPropertyData
			// ...
			
			widgetSize = 30;

			// ...

			if(0 != widgetSize)
			{
				newWidgetRect.setLeft(newWidgetRect.right() - widgetSize);
				optionalWidget->setGeometry(newWidgetRect);
				optionalWidget->show();

				// if this widget isn't overlayed we should modify rect for tree view cell to be drawn in.
				if(!data->GetOptionalWidgetOverlay())
				{
					option->rect.setRight(newWidgetRect.left());
				}
			}
			else
			{
				optionalWidget->hide();
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
