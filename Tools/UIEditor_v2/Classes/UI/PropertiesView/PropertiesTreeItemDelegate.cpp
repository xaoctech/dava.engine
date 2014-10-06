#include "PropertiesTreeItemDelegate.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QPainter>
#include <QStylePainter>
#include <QApplication>
#include <QToolButton>
#include "QtControls/lineeditext.h"

#include "DAVAEngine.h"
#include "QtControls/Vector2DEdit.h"
#include "UIControls/BaseProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "ItemDelegateForVector2.h"
#include "ItemDelegateForPropertyEnum.h"
#include "PropertiesTreeModel.h"
#include "ItemDelegateForString.h"
#include "ItemDelegateForFilePath.h"
#include "ItemDelegateForColor.h"
#include "ItemDelegateForInteger.h"
#include "ItemDelegateForFloat.h"

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    propertyItemDelegates[BaseProperty::TYPE_ENUM] = new ItemDelegateForPropertyEnum(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR2] = new ItemDelegateForVector2(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_STRING] = new ItemDelegateForString(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_WIDE_STRING] = new ItemDelegateForString(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FILEPATH] = new ItemDelegateForFilePath();
    variantTypeItemDelegates[DAVA::VariantType::TYPE_COLOR] = new ItemDelegateForColor(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT32] = new ItemDelegateForInteger(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT64] = new ItemDelegateForInteger(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT32] = new ItemDelegateForInteger(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT64] = new ItemDelegateForInteger(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FLOAT] = new ItemDelegateForFloat(this);
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
}

void PropertiesTreeItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    PropertyAbstractEditor *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        currentDelegate->paint(painter, opt, index);
    }

    //return QStyledItemDelegate::paint(painter, opt, index);
    QStyledItemDelegate::paint(painter, opt, index);
}

QSize PropertiesTreeItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *PropertiesTreeItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    PropertyAbstractEditor *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        return currentDelegate->createEditorWidget(parent, option, index);
    }

    if (index.data(Qt::EditRole).type() == QVariant::Bool)
        return NULL;

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertiesTreeItemDelegate::setEditorData( QWidget *editor, const QModelIndex & index ) const
{
    PropertyAbstractEditor *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
        return currentDelegate->setEditorData(editor, index);

    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertiesTreeItemDelegate::setModelData(QWidget * editor, QAbstractItemModel *model, const QModelIndex & index) const
{
    PropertyAbstractEditor *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        currentDelegate->setModelData(editor, model, index);
        return;
    }

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit && !lineEdit->isModified())
        return;

    QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertiesTreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);

//     if(NULL != editor)
//     {
//         QRect r = option.rect;
//         r.setWidth(r.width() - r.height());
//         editor->setGeometry(r);
//     }

}

bool PropertiesTreeItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
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
    }

    QVariant editValue = index.data(Qt::EditRole);
    if (editValue.userType() == QMetaTypeId<DAVA::VariantType>::qt_metatype_id())
    {
        DAVA::VariantType variantType = editValue.value<DAVA::VariantType>();
        QMap<DAVA::VariantType::eVariantType, PropertyAbstractEditor *>::const_iterator var_iter = variantTypeItemDelegates.find(variantType.GetType());
        if (var_iter != variantTypeItemDelegates.end())
            return var_iter.value();
    }
    else
    {
        QMap<QVariant::Type, PropertyAbstractEditor *>::const_iterator iter = qvariantItemDelegates.find(editValue.type());
        if (iter != qvariantItemDelegates.end())
            return iter.value();
    }

    return NULL;
}

void PropertiesTreeItemDelegate::emitCommitData(QWidget * editor)
{
    emit commitData(editor);
}

void PropertiesTreeItemDelegate::emitCloseEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
    emit closeEditor(editor, hint);
}


