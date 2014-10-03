#ifndef __ITEMDELEGATEFORSTRING_H__
#define __ITEMDELEGATEFORSTRING_H__

#include "PropertyAbstractEditor.h"

class ItemDelegateForString: public PropertyAbstractEditor
{
    Q_OBJECT
public:
    explicit ItemDelegateForString(PropertiesTreeItemDelegate *delegate);
    ~ItemDelegateForString();

    virtual void addEditorWidgets(QWidget *parent, const QModelIndex &index) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
private slots:
    void OnValueChanged();
};


#endif // __ITEMDELEGATEFORSTRING_H__
