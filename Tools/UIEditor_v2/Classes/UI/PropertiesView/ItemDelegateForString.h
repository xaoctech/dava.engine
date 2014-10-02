#ifndef __ITEMDELEGATEFORSTRING_H__
#define __ITEMDELEGATEFORSTRING_H__

#include "PropertyAbstractEditor.h"

class ItemDelegateForString: public PropertyAbstractEditor
{
    //    Q_OBJECT
public:
    explicit ItemDelegateForString();
    ~ItemDelegateForString();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
};


#endif // __ITEMDELEGATEFORSTRING_H__
