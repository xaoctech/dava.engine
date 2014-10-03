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

    virtual void addEditorWidgets( QWidget * parent, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
private slots:
    void OnValueChanged();
};

#endif // __ITEMDELEGATEFORFLOAT_H__