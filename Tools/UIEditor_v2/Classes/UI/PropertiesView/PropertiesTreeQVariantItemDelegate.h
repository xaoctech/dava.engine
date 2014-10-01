#ifndef __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
#define __PROPERTIESTREEQVARIANTITEMDELEGATE_H__

#include <QStyledItemDelegate>

class PropertiesTreeQVariantItemDelegate: public QStyledItemDelegate
{
    friend class PropertiesTreeItemDelegate;
    Q_OBJECT
public:
    explicit PropertiesTreeQVariantItemDelegate(QObject *parent = NULL);
    ~PropertiesTreeQVariantItemDelegate();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
public:
    bool IsEditorDataModified(QWidget *editor) const;
    //virtual bool eventFilter(QObject *object, QEvent *event);

private slots:
    void OnCommitData(QWidget *editor);
    void OnCloseEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
};

#endif // __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
