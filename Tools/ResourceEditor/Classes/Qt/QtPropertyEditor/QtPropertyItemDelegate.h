#ifndef __QT_PROPERY_ITEM_DELEGATE_H__
#define __QT_PROPERY_ITEM_DELEGATE_H__

#include <QStyledItemDelegate>

class QtPropertyData;

class QtPropertyItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QtPropertyItemDelegate(QWidget *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;

public slots:
	void collapse(const QModelIndex & index);
	void expand(const QModelIndex & index);

protected:
	void recalcOptionalWidgets(const QModelIndex &index, QStyleOptionViewItem *option) const;
	void hideAllChildOptionalWidgets(QtPropertyData* data);
};

#endif // __QT_PROPERY_ITEM_DELEGATE_H__
