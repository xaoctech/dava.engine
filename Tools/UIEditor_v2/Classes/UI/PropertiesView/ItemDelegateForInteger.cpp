#include "ItemDelegateForInteger.h"
#include <QSpinBox>
#include "FileSystem/VariantType.h"
#include "PropertiesTreeModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

ItemDelegateForInteger::ItemDelegateForInteger(PropertiesTreeItemDelegate *delegate)
    : itemDelegate(delegate)
{

}

ItemDelegateForInteger::~ItemDelegateForInteger()
{

}

QWidget * ItemDelegateForInteger::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    QSpinBox *editor = new QSpinBox(parent);
    connect(editor, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));
    return editor;
}

void ItemDelegateForInteger::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QSpinBox *editor = static_cast<QSpinBox *>(rawEditor);

    DAVA::VariantType variant = index.data(DAVA::VariantTypeEditRole).value<DAVA::VariantType>();
    switch (variant.GetType())
    {
    case DAVA::VariantType::TYPE_INT32:
        editor->setValue(variant.AsInt32());
        break;
    case DAVA::VariantType::TYPE_INT64:
        editor->setValue(variant.AsInt64());
        break;
    case DAVA::VariantType::TYPE_UINT32:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt32());
        break;
    case DAVA::VariantType::TYPE_UINT64:
        editor->setMinimum(0);
        editor->setValue(variant.AsUInt64());
        break;
    default:
        break;
    }
    PropertyAbstractEditor::SetValueModified(editor, false);
}

void ItemDelegateForInteger::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    QSpinBox *editor = static_cast<QSpinBox *>(rawEditor);
    if (!PropertyAbstractEditor::IsValueModified(editor))
        return;

    DAVA::VariantType variantType = index.data(DAVA::VariantTypeEditRole).value<DAVA::VariantType>();

    switch (variantType.GetType())
    {
    case DAVA::VariantType::TYPE_INT32:
        variantType.SetInt32(editor->value());
        break;
    case DAVA::VariantType::TYPE_INT64:
        variantType.SetInt64(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT32:
        variantType.SetUInt32(editor->value());
        break;
    case DAVA::VariantType::TYPE_UINT64:
        variantType.SetUInt64(editor->value());
        break;
    default:
        break;
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    model->setData(index, variant, DAVA::VariantTypeEditRole);
}

void ItemDelegateForInteger::OnValueChanged()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->NeedCommitData(editor);
}
