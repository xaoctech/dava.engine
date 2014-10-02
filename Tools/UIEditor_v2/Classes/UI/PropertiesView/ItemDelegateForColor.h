#ifndef __ITEMDELEGATEFORCOLOR_H__
#define __ITEMDELEGATEFORCOLOR_H__


#include "PropertyAbstractEditor.h"

class ItemDelegateForColor: public PropertyAbstractEditor
{
    //    Q_OBJECT
public:
    explicit ItemDelegateForColor();
    ~ItemDelegateForColor();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
};


#endif // __ITEMDELEGATEFORCOLOR_H__
