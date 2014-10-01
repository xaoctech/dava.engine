#include "ItemDelegateForQVector2D.h"
#include <QItemEditorFactory>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include "QtControls/Vector2DEdit.h"

ItemDelegateForQVector2D::ItemDelegateForQVector2D( QObject *parent /*= NULL*/ )
    : QStyledItemDelegate(parent)
{
    connect(this, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
    connect(this, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this, SLOT(OnCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
}

ItemDelegateForQVector2D::~ItemDelegateForQVector2D()
{
    disconnect(this, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
    disconnect(this, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this, SLOT(OnCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
}

QWidget * ItemDelegateForQVector2D::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    if (index.data(Qt::EditRole).type() != QVariant::Vector2D)
    {
        return NULL;
    }
    return new Vector2DEdit(parent);
}

void ItemDelegateForQVector2D::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    if (index.data(Qt::EditRole).type() != QVariant::Vector2D)
    {
        return;
    }

    Vector2DEdit *vectorEditor = static_cast<Vector2DEdit *>(editor);
    vectorEditor->setVector2D(index.data(Qt::EditRole).value<QVector2D>());
}

void ItemDelegateForQVector2D::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (index.data(Qt::EditRole).type() != QVariant::Vector2D)
    {
        return;
    }

    Vector2DEdit *vectorEditor = static_cast<Vector2DEdit *>(editor);
    if (!vectorEditor->isModified())
        return;

    QVector2D vector = vectorEditor->vector2D();
    if (model->setData(index, vector, Qt::EditRole))
    {

    }
    else
    {
        int t=0;
    }
}

void ItemDelegateForQVector2D::OnCommitData( QWidget *editor )
{
    emit commitData(editor);
}

void ItemDelegateForQVector2D::OnCloseEditor( QWidget *editor, QAbstractItemDelegate::EndEditHint hint )
{
    emit closeEditor(editor);
}
