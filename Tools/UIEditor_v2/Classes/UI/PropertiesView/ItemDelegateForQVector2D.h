#ifndef __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
#define __PROPERTIESTREEQVARIANTITEMDELEGATE_H__

#include <QWidget>
#include "PropertyAbstractEditor.h"

class ItemDelegateForQVector2D: public PropertyAbstractEditor
{
//    Q_OBJECT
public:
    explicit ItemDelegateForQVector2D();
    ~ItemDelegateForQVector2D();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
};

#endif // __PROPERTIESTREEQVARIANTITEMDELEGATE_H__
