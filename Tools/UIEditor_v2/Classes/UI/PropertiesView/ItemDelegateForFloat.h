#ifndef __ITEMDELEGATEFORFLOAT_H__
#define __ITEMDELEGATEFORFLOAT_H__

#include "DAVAEngine.h"

#include "PropertyAbstractEditor.h"
class PropertiesTreeItemDelegate;

class ItemDelegateForFloat: public PropertyAbstractEditor
{
    Q_OBJECT
public:
    explicit ItemDelegateForFloat(PropertiesTreeItemDelegate *delegate);
    ~ItemDelegateForFloat();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
private slots:
    void OnValueChanged();
private:
    PropertiesTreeItemDelegate *itemDelegate;
};

#endif // __ITEMDELEGATEFORFLOAT_H__