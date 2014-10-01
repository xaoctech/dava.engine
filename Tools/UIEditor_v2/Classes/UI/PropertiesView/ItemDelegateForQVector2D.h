#ifndef __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
#define __PROPERTIESTREEQVARIANTITEMDELEGATE_H__

#include <QStyledItemDelegate>

class ItemDelegateForQVector2D: public QStyledItemDelegate
{
    friend class PropertiesTreeItemDelegate;
    Q_OBJECT
public:
    explicit ItemDelegateForQVector2D(QObject *parent = NULL);
    ~ItemDelegateForQVector2D();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
public:

private slots:
    void OnCommitData(QWidget *editor);
    void OnCloseEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
};

#endif // __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
