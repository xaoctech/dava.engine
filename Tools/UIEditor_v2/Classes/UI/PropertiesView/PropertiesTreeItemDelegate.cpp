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
    , currentDelegate(NULL)
{
    propertyItemDelegates[BaseProperty::TYPE_ENUM] = new ItemDelegateForPropertyEnum(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR2] = new ItemDelegateForVector2();
    variantTypeItemDelegates[DAVA::VariantType::TYPE_STRING] = new ItemDelegateForString();
    variantTypeItemDelegates[DAVA::VariantType::TYPE_WIDE_STRING] = new ItemDelegateForString();
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FILEPATH] = new ItemDelegateForFilePath();
    variantTypeItemDelegates[DAVA::VariantType::TYPE_COLOR] = new ItemDelegateForColor();
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
//     if (index.column() == 1)
//     {
// 
//         QStyleOptionToolButton buttonOption;
//         buttonOption.subControls = QStyle::SC_ToolButton;
//         buttonOption.activeSubControls = QStyle::SC_ToolButton;
//         buttonOption.state = option.state;
//         QRect buttonrect = opt.rect;
//         buttonrect.setWidth(buttonrect.height());
//         buttonrect.moveRight(opt.rect.right());
//         buttonOption.features = QStyleOptionToolButton::None;
//         buttonOption.rect = buttonrect;
//         buttonOption.arrowType = Qt::NoArrow;
//         buttonOption.toolButtonStyle = Qt::ToolButtonIconOnly;
//         buttonOption.icon = QIcon(":/Icons/editclear.png");
//         buttonOption.iconSize = buttonOption.rect.size();
// 
//         QApplication::style()->drawComplexControl(QStyle::CC_ToolButton, &buttonOption, painter);
//         opt.rect.setRight(buttonOption.rect.left());
//     }

    return QStyledItemDelegate::paint(painter, opt, index);
}

QSize PropertiesTreeItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *PropertiesTreeItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        return currentDelegate->createEditor(parent, option, index);
    }

    if (index.data(Qt::EditRole).type() == QVariant::Bool)
        return NULL;

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

        DAVA::VariantType variantType = index.data(DAVA::VariantTypeEditRole).value<DAVA::VariantType>();
        QMap<DAVA::VariantType::eVariantType, PropertyAbstractEditor *>::const_iterator var_iter = variantTypeItemDelegates.find(variantType.GetType());
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


