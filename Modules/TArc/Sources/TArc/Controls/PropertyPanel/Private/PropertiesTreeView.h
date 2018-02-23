#pragma once

#include <QTreeView>

namespace DAVA
{
class ReflectedPropertyModel;
class PropertiesTreeView : public QTreeView
{
public:
    PropertiesTreeView(QWidget* parent);

    void SetFavoritesEditMode(bool inEditMode);
    bool IsInFavoritesEditMode() const;

    int GetIndexWidth(const QModelIndex& index) const;

    void setModel(QAbstractItemModel* model) override;

protected:
    void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) override;
    void rowsInserted(const QModelIndex& index, int start, int end) override;

private:
    class PropertiesTreeViewPrivate;
    class PropertiesHeaderView;
    PropertiesHeaderView* headerView = nullptr;
    bool isInFavoritesEdit = false;
    ReflectedPropertyModel* propertiesModel = nullptr;

    Q_DECLARE_PRIVATE(PropertiesTreeView)
    Q_DISABLE_COPY(PropertiesTreeView)
};
} // namespace DAVA