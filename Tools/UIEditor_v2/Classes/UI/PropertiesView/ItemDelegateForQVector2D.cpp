#include "ItemDelegateForQVector2D.h"
#include <QItemEditorFactory>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include "QtControls/Vector2DEdit.h"

ItemDelegateForQVector2D::ItemDelegateForQVector2D()
    : PropertyAbstractEditor()
{
}

ItemDelegateForQVector2D::~ItemDelegateForQVector2D()
{
}

QWidget * ItemDelegateForQVector2D::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    return new Vector2DEdit(parent);
}

void ItemDelegateForQVector2D::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = static_cast<Vector2DEdit *>(editor);
    vectorEditor->setVector2D(index.data(Qt::EditRole).value<QVector2D>());
}

void ItemDelegateForQVector2D::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    Vector2DEdit *vectorEditor = static_cast<Vector2DEdit *>(editor);
    if (!vectorEditor->isModified())
        return;

    QVector2D vector = vectorEditor->vector2D();
    model->setData(index, vector, Qt::EditRole);
}
