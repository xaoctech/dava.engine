#ifndef __PROPERTIESTREEITEMDELEGATE_H__
#define __PROPERTIESTREEITEMDELEGATE_H__

#include <QWidget>
#include <QVector2D>
#include <QLineEdit>
#include <QStyledItemDelegate>
class PropertiesTreeQVariantItemDelegate;

class PropertiesTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PropertiesTreeItemDelegate(QObject *parent = NULL);
    ~PropertiesTreeItemDelegate();

    virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QAbstractItemDelegate * GetCustomItemDelegateForIndex(const QModelIndex & index) const;
    bool registerItemDelegate( QAbstractItemDelegate *delegate ){ return false; }
private slots:
    void OnCommitData(QWidget *editor);
    void OnCloseEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);
private:
    QMap<QVariant::Type, QAbstractItemDelegate *> qvariantItemDelegates;
    mutable QAbstractItemDelegate *currentDelegate;
};
#endif // __PROPERTIESTREEITEMDELEGATE_H__
