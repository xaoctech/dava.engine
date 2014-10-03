#include "ItemDelegateForInteger.h"
#include <QSpinBox>
#include <QLayout>
#include "FileSystem/VariantType.h"
#include "PropertiesTreeModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"

ItemDelegateForInteger::ItemDelegateForInteger(PropertiesTreeItemDelegate *delegate)
    : PropertyAbstractEditor(delegate)
{

}

ItemDelegateForInteger::~ItemDelegateForInteger()
{

}

void ItemDelegateForInteger::addEditorWidgets( QWidget *parent, const QModelIndex &index ) const
{
    QSpinBox *spinBox = new QSpinBox(parent);
    spinBox->setObjectName(QString::fromUtf8("spinBox"));
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));
    parent->layout()->addWidget(spinBox);

    PropertyAbstractEditor::addEditorWidgets(parent, index);
}

void ItemDelegateForInteger::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QSpinBox *editor = rawEditor->findChild<QSpinBox*>("spinBox");

    editor->blockSignals(true);
    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
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
    editor->blockSignals(false);
    PropertyAbstractEditor::SetValueModified(editor, false);
}

bool ItemDelegateForInteger::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (PropertyAbstractEditor::setModelData(rawEditor, model, index))
        return true;

    QSpinBox *editor = rawEditor->findChild<QSpinBox*>("spinBox");

    DAVA::VariantType variantType = index.data(Qt::EditRole).value<DAVA::VariantType>();

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

    return model->setData(index, variant, Qt::EditRole);
}

void ItemDelegateForInteger::OnValueChanged()
{
    QWidget *spinBox = qobject_cast<QWidget *>(sender());
    if (!spinBox)
        return;

    QWidget *editor = spinBox->parentWidget();
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
