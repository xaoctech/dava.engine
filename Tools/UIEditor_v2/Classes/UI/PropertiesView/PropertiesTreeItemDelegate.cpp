#include "PropertiesTreeItemDelegate.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include "QtControls/lineeditext.h"

#include "DAVAEngine.h"
#include "QtControls/Vector2DEdit.h"
#include "UIControls/BaseProperty.h"
#include "ItemDelegateForQVector2D.h"

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , currentDelegate(NULL)
{
    qvariantItemDelegates[QVariant::Vector2D] = new ItemDelegateForQVector2D(this);
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
}

void PropertiesTreeItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if (currentDelegate)
        return currentDelegate->paint(painter, option, index);

    return QStyledItemDelegate::paint(painter, option, index);
}

QSize PropertiesTreeItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    if (currentDelegate)
        return currentDelegate->sizeHint(option, index);

    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *PropertiesTreeItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QAbstractItemDelegate *itemDelegate = GetCustomItemDelegateForIndex(index);
    if (itemDelegate)
    {
        currentDelegate = itemDelegate;
        connect(currentDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
        connect(currentDelegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this, SLOT(OnCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
        return itemDelegate->createEditor(parent, option, index);
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertiesTreeItemDelegate::setEditorData( QWidget *editor, const QModelIndex & index ) const
{
    if (currentDelegate)
        return currentDelegate->setEditorData(editor, index);

    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertiesTreeItemDelegate::setModelData(QWidget * editor, QAbstractItemModel *model, const QModelIndex & index) const
{
    if (currentDelegate)
        return currentDelegate->setModelData(editor, model, index);

    QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertiesTreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (currentDelegate)
        return currentDelegate->updateEditorGeometry(editor, option, index);

    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

bool PropertiesTreeItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (currentDelegate)
        return currentDelegate->editorEvent(event, model, option, index);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QAbstractItemDelegate * PropertiesTreeItemDelegate::GetCustomItemDelegateForIndex( const QModelIndex & index ) const
{
    QMap<QVariant::Type, QAbstractItemDelegate *>::const_iterator iter = qvariantItemDelegates.find(index.data(Qt::EditRole).type());
    if (iter != qvariantItemDelegates.end())
        return iter.value();
    return NULL;
}

void PropertiesTreeItemDelegate::OnCommitData(QWidget *editor)
{
    emit commitData(editor);
}

void PropertiesTreeItemDelegate::OnCloseEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    emit closeEditor(editor, hint);
    disconnect(currentDelegate, SIGNAL(commitData(QWidget *)), this, SLOT(OnCommitData(QWidget *)));
    disconnect(currentDelegate, SIGNAL(closeEditor(QWidget *, QAbstractItemDelegate::EndEditHint)), this, SLOT(OnCloseEditor(QWidget *, QAbstractItemDelegate::EndEditHint)));
    currentDelegate = NULL;
}

