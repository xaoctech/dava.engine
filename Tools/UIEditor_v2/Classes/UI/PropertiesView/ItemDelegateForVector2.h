#ifndef __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
#define __PROPERTIESTREEQVARIANTITEMDELEGATE_H__

#include <QWidget>
#include "PropertyAbstractEditor.h"

class ItemDelegateForVector2: public PropertyAbstractEditor
{
//    Q_OBJECT
public:
    explicit ItemDelegateForVector2();
    ~ItemDelegateForVector2();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
};

#endif // __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
