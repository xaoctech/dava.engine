#include "TArc/Controls/PropertyPanel/Private/PropertiesTreeView.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Utils/Utils.h"

#include <QHeaderView>
#include <QPainter>
#include <QScrollBar>
#include <QtWidgets/private/qheaderview_p.h>
#include <QtWidgets/private/qtreeview_p.h>

namespace DAVA
{
class PropertiesTreeView::PropertiesHeaderView : public QHeaderView
{
public:
    static const int FavoritesStarSpaceWidth;
    PropertiesHeaderView(Qt::Orientation orientation, QWidget* parent)
        : QHeaderView(orientation, parent)
    {
    }

    void SetFavoritesEditMode(bool isActive)
    {
        isInFavoritesEdit = isActive;
        setOffset(isInFavoritesEdit == true ? -FavoritesStarSpaceWidth : 0);
    }

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override
    {
        QRect r = rect;
        if (isInFavoritesEdit == true && logicalIndex == 0)
        {
            r.setX(0);
        }

        QHeaderView::paintSection(painter, r, logicalIndex);
    }

private:
    bool isInFavoritesEdit = false;
};

const int PropertiesTreeView::PropertiesHeaderView::FavoritesStarSpaceWidth = 20;

PropertiesTreeView::PropertiesTreeView(QWidget* parent)
    : QTreeView(parent)
{
    headerView = new PropertiesHeaderView(Qt::Horizontal, this);
    setHeader(headerView);
    headerView->setStretchLastSection(true);
    setMouseTracking(true);
}

void PropertiesTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    propertiesModel = qobject_cast<ReflectedPropertyModel*>(model);
}

void PropertiesTreeView::SetFavoritesEditMode(bool inEditMode)
{
    isInFavoritesEdit = inEditMode;
    headerView->SetFavoritesEditMode(inEditMode);
    viewport()->update();
    headerView->update();
}

bool PropertiesTreeView::IsInFavoritesEditMode() const
{
    return isInFavoritesEdit;
}

int PropertiesTreeView::GetIndexWidth(const QModelIndex& index) const
{
    QTreeViewPrivate* d = reinterpret_cast<QTreeViewPrivate*>(qGetPtrHelper(d_ptr));
    int item = d->viewIndex(index);
    if (item == -1)
    {
        return 0;
    }

    int indention = d->indentationForItem(item);

    if (isFirstColumnSpanned(index.row(), index.parent()) == true)
    {
        return columnWidth(0) + columnWidth(1) - indention;
    }

    int width = columnWidth(index.row());
    if (index.row() == 0)
    {
        width -= indention;
    }

    return width;
}

void PropertiesTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const
{
    QTreeView::drawRow(painter, options, index);

    QColor gridColor = options.palette.color(QPalette::Normal, QPalette::Mid);

    painter->save();
    // draw horizontal bottom line
    painter->setPen(gridColor);
    painter->drawLine(options.rect.bottomLeft(), options.rect.bottomRight());

    // draw vertical line
    bool isSelected = options.state & QStyle::State_Selected;
    bool isSpanned = isFirstColumnSpanned(index.row(), index.parent());
    if (isSelected == false && isSpanned == false)
    {
        if (headerView != nullptr && headerView->count() > 1)
        {
            int sz = headerView->sectionSize(0) - headerView->offset();
            QScrollBar* scroll = horizontalScrollBar();
            if (scroll != nullptr)
            {
                sz -= scroll->value();
            }

            QPoint p1 = options.rect.topLeft();
            QPoint p2 = options.rect.bottomLeft();

            p1.setX(p1.x() + sz - 1);
            p2.setX(p2.x() + sz - 1);

            painter->setPen(gridColor);
            painter->drawLine(p1, p2);
        }
    }

    painter->restore();

    if (isInFavoritesEdit == true)
    {
        bool isFavorite = propertiesModel->IsFavorite(index);
        QRect iconRect = QRect(options.rect.x(), options.rect.y(), PropertiesTreeView::PropertiesHeaderView::FavoritesStarSpaceWidth, options.rect.height());

        if (isFavorite)
        {
            SharedIcon(":/QtIcons/star.png").paint(painter, iconRect);
        }
        else if (propertiesModel->IsInFavoriteHierarchy(index) == false)
        {
            SharedIcon(":/QtIcons/star_empty.png").paint(painter, iconRect);
        }
    }
}

void PropertiesTreeView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isInFavoritesEdit)
    {
        QPoint normalizedEventPos = event->pos();
        normalizedEventPos.rx() += -headerView->offset();
        QModelIndex index = indexAt(normalizedEventPos);

        if (index.isValid() && index.column() == 0)
        {
            QRect rect = visualRect(index);
            rect.setX(0);
            rect.setWidth(-headerView->offset());

            if (rect.contains(event->pos()))
            {
                if (propertiesModel->IsFavorite(index))
                {
                    propertiesModel->RemoveFavorite(index);
                }
                else if (propertiesModel->IsInFavoriteHierarchy(index) == false)
                {
                    propertiesModel->AddFavorite(index);
                }

                viewport()->update();
            }
        }
    }
    QTreeView::mouseReleaseEvent(event);
}

bool PropertiesTreeView::edit(const QModelIndex& index, EditTrigger trigger, QEvent* event)
{
    if (trigger == SelectedClicked)
    {
        return QTreeView::edit(index, QAbstractItemView::EditKeyPressed, event);
    }

    return QTreeView::edit(index, trigger, event);
}

void PropertiesTreeView::rowsInserted(const QModelIndex& index, int start, int end)
{
    QTreeView::rowsInserted(index, start, end);
    for (int i = start; i <= end; ++i)
    {
        QModelIndex insertedIndex = index.child(i, 0);
        setFirstColumnSpanned(i, index, propertiesModel->IsEditorSpanned(insertedIndex));
    }
}

} // namespace DAVA
