#ifndef __ITEMDELEGATEFORPROPERTYENUM_H__
#define __ITEMDELEGATEFORPROPERTYENUM_H__

#include "PropertyAbstractEditor.h"
#include <QComboBox>
#include <QAbstractItemDelegate>
#include <QPointer>

class PropertiesTreeItemDelegate;

class ItemDelegateForPropertyEnum: public PropertyAbstractEditor
{
    Q_OBJECT
public:
    ItemDelegateForPropertyEnum(PropertiesTreeItemDelegate *delegate);
    ~ItemDelegateForPropertyEnum();

    virtual void addEditorWidgets(QWidget *parent, const QModelIndex &index) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;

private slots:
    void OnCurrentIndexChanged();
};

#endif // __ITEMDELEGATEFORPROPERTYENUM_H__
