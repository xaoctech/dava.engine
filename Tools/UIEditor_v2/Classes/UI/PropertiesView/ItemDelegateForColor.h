#ifndef __ITEMDELEGATEFORCOLOR_H__
#define __ITEMDELEGATEFORCOLOR_H__

#include "PropertyAbstractEditor.h"

class QToolButton;

class ItemDelegateForColor: public PropertyAbstractEditor
{
    Q_OBJECT
public:
    explicit ItemDelegateForColor(PropertiesTreeItemDelegate *delegate);
    ~ItemDelegateForColor();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual QList<QAction *> enumEditorActions(QWidget *parent, const QModelIndex &index) const;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
private slots:
    void chooseColorClicked();
};


#endif // __ITEMDELEGATEFORCOLOR_H__
