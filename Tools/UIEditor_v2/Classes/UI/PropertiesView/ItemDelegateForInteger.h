#ifndef __ITEMDELEGATEFORINTEGER_H__
#define __ITEMDELEGATEFORINTEGER_H__

#include "PropertyAbstractEditor.h"
class PropertiesTreeItemDelegate;

class ItemDelegateForInteger: public PropertyAbstractEditor
{
    Q_OBJECT
public:
    explicit ItemDelegateForInteger(PropertiesTreeItemDelegate *delegate);
    ~ItemDelegateForInteger();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
private slots:
        void OnValueChanged();
private:
    PropertiesTreeItemDelegate *itemDelegate;
};


#endif // __ITEMDELEGATEFORINTEGER_H__