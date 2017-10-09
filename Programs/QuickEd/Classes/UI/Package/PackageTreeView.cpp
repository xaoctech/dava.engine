#include "Classes/UI/Package/PackageTreeView.h"
#include <QPainter>
#include <QHeaderView>
#include <QApplication>

#include <QStyleOptionButton>

PackageTreeView::PackageTreeView(QWidget* parent /*= NULL*/)
    : QTreeView(parent)
{
    connect(this, &QTreeView::model)
}

PackageTreeView::~PackageTreeView()
{
}

void PackageTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QTreeView::drawRow(painter, option, index);

    if (index.flags() & Qt::ItemIsUserCheckable)
    {
        QVariant value = index.data(Qt::CheckStateRole);
        if (value.isValid())
        {
            QStyleOptionViewItem checkBoxOption = CreateCheckBoxOption(option, value);

            style()->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &checkBoxOption, painter, this);
        }
    }
}

QStyleOptionViewItem PackageTreeView::CreateCheckBoxOption(const QStyleOptionViewItem& option, const QVariant& checked) const
{
    QStyleOptionViewItem checkBoxOption(option);
    checkBoxOption.rect = checkBoxOption.rect;

    checkBoxOption.displayAlignment = Qt::AlignLeft | Qt::AlignTop;
    checkBoxOption.state = checkBoxOption.state & ~QStyle::State_HasFocus;

    switch (checked.toInt())
    {
    case Qt::Unchecked:
        checkBoxOption.state |= QStyle::State_Off;
        break;
    case Qt::PartiallyChecked:
        checkBoxOption.state |= QStyle::State_NoChange;
        break;
    case Qt::Checked:
        checkBoxOption.state |= QStyle::State_On;
        break;
    }
    checkBoxOption.features |= QStyleOptionViewItem::HasCheckIndicator;

    QRect checkBoxRect = style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &checkBoxOption, this);
    checkBoxOption.rect = checkBoxRect;
}

void PackageTreeView::mouseReleaseEvent(QMouseEvent* event)
{
}
