#include "FilePathPropertyDelegate.h"

FilePathPropertyDelegate::FilePathPropertyDelegate()
{

}

FilePathPropertyDelegate::~FilePathPropertyDelegate()
{

}

QWidget * FilePathPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    return NULL;
}

void FilePathPropertyDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{

}

bool FilePathPropertyDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    return BasePropertyDelegate::setModelData(editor, model, index);
}
