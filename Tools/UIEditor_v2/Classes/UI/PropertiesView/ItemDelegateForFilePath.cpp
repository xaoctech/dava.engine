#include "ItemDelegateForFilePath.h"

ItemDelegateForFilePath::ItemDelegateForFilePath()
{

}

ItemDelegateForFilePath::~ItemDelegateForFilePath()
{

}

QWidget * ItemDelegateForFilePath::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    return NULL;
}

void ItemDelegateForFilePath::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{

}

bool ItemDelegateForFilePath::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    return PropertyAbstractEditor::setModelData(editor, model, index);
}
