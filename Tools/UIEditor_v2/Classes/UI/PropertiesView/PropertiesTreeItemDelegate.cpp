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
#include "ItemDelegateForPropertyEnum.h"

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , currentDelegate(NULL)
{
    qvariantItemDelegates[QVariant::Vector2D] = new ItemDelegateForQVector2D();
    propertyItemDelegates[BaseProperty::TYPE_ENUM] = new ItemDelegateForPropertyEnum(this);
    propertyItemDelegates[BaseProperty::TYPE_FLAGS] = new ItemDelegateForPropertyEnum(this);
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
}

void PropertiesTreeItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
//     if (currentDelegate)
//         return currentDelegate->paint(painter, option, index);

    return QStyledItemDelegate::paint(painter, option, index);
}

QSize PropertiesTreeItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
//     if (currentDelegate)
//         return currentDelegate->sizeHint(option, index);

    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *PropertiesTreeItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        return currentDelegate->createEditor(parent, option, index);
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

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit && !lineEdit->isModified())
        return;

//     QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
//     if (comboBox && !lineEdit->isModified())
//         return;

    QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertiesTreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//     if (currentDelegate)
//         return currentDelegate->updateEditorGeometry(editor, option, index);

    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

bool PropertiesTreeItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
//     if (currentDelegate)
//         return currentDelegate->editorEvent(event, model, option, index);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

PropertyAbstractEditor * PropertiesTreeItemDelegate::GetCustomItemDelegateForIndex( const QModelIndex & index ) const
{
    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    if (property)
    {
        QMap<BaseProperty::ePropertyType, PropertyAbstractEditor *>::const_iterator prop_iter = propertyItemDelegates.find(property->GetType());
        if (prop_iter != propertyItemDelegates.end())
            return prop_iter.value();

        QMap<DAVA::VariantType::eVariantType, PropertyAbstractEditor *>::const_iterator var_iter = variantTypeItemDelegates.find(property->GetValue().GetType());
        if (var_iter != variantTypeItemDelegates.end())
            return var_iter.value();
    }

    QMap<QVariant::Type, PropertyAbstractEditor *>::const_iterator iter = qvariantItemDelegates.find(index.data(Qt::EditRole).type());
    if (iter != qvariantItemDelegates.end())
        return iter.value();

    return NULL;
}

void PropertiesTreeItemDelegate::NeedCommitData( QWidget * editor )
{
    emit commitData(editor);
}

void PropertiesTreeItemDelegate::NeedCommitDataAndCloseEditor( QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
    emit commitData(editor);
    emit closeEditor(editor, hint);
}


