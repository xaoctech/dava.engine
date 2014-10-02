#include "ItemDelegateForString.h"
#include <QLineEdit>
#include "DAVAEngine.h"
#include "PropertiesTreeModel.h"
#include "Utils/QtDavaConvertion.h"

ItemDelegateForString::ItemDelegateForString()
{

}

ItemDelegateForString::~ItemDelegateForString()
{

}

QWidget * ItemDelegateForString::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    return new QLineEdit(parent);
}

void ItemDelegateForString::setEditorData( QWidget *rawEditor, const QModelIndex & index ) const 
{
    QLineEdit *editor = static_cast<QLineEdit *>(rawEditor);

    DAVA::VariantType variant = index.data(DAVA::VariantTypeEditRole).value<DAVA::VariantType>();
    QString stringValue;
    if (variant.GetType() == DAVA::VariantType::TYPE_STRING)
    {
        stringValue = StringToQString(variant.AsString());
    }
    else
    {
        stringValue = WideStringToQString(variant.AsWideString());
    }
    editor->setText(stringValue);
}

void ItemDelegateForString::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    QLineEdit *editor = static_cast<QLineEdit *>(rawEditor);
    if (!editor->isModified())
        return;

    DAVA::VariantType variantType = index.data(DAVA::VariantTypeEditRole).value<DAVA::VariantType>();

    if (variantType.GetType() == DAVA::VariantType::TYPE_STRING)
    {
        variantType.SetString(QStringToString(editor->text()));
    }
    else
    {
        variantType.SetWideString(QStringToWideString(editor->text()));
    }

    QVariant variant;
    variant.setValue<DAVA::VariantType>(variantType);

    model->setData(index, variant, DAVA::VariantTypeEditRole);
}
