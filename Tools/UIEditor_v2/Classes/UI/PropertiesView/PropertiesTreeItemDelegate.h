#ifndef __PROPERTIESTREEITEMDELEGATE_H__
#define __PROPERTIESTREEITEMDELEGATE_H__

#include <QWidget>
#include <QVector2D>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include "UIControls/BaseProperty.h"
#include "FileSystem/VariantType.h"
class PropertyAbstractEditor;

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

    PropertyAbstractEditor * GetCustomItemDelegateForIndex(const QModelIndex & index) const;

    void NeedCommitData(QWidget * editor);
    void NeedCommitDataAndCloseEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint = NoHint);

private:
    QMap<QVariant::Type, PropertyAbstractEditor *> qvariantItemDelegates;
    QMap<BaseProperty::ePropertyType, PropertyAbstractEditor *> propertyItemDelegates;
    QMap<DAVA::VariantType::eVariantType, PropertyAbstractEditor *> variantTypeItemDelegates;
    
    mutable PropertyAbstractEditor *currentDelegate;
};
#endif // __PROPERTIESTREEITEMDELEGATE_H__
