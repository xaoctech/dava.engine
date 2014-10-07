#ifndef __ITEMDELEGATEFORFILEPATH_H__
#define __ITEMDELEGATEFORFILEPATH_H__

#include "BasePropertyDelegate.h"

class FilePathPropertyDelegate: public BasePropertyDelegate
{
    //    Q_OBJECT
public:
    explicit FilePathPropertyDelegate();
    ~FilePathPropertyDelegate();

    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual bool setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
};


#endif // __ITEMDELEGATEFORFILEPATH_H__
