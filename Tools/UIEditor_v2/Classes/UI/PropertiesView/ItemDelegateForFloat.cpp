#include "ItemDelegateForFloat.h"
#include "FileSystem/VariantType.h"
#include "PropertiesTreeModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include <QLineEdit>
#include <QDoubleValidator>


ItemDelegateForFloat::ItemDelegateForFloat( PropertiesTreeItemDelegate *delegate )
    : itemDelegate(delegate)
{

}

ItemDelegateForFloat::~ItemDelegateForFloat()
{

}

QWidget * ItemDelegateForFloat::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    QLineEdit *editor = new QLineEdit(parent);
    connect(editor, SIGNAL(textChanged(const QString &)), this, SLOT(OnValueChanged()));
    editor->setValidator( new QDoubleValidator(-999999.0, 999999.0, 6, editor) );
    return editor;
}

void ItemDelegateForFloat::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QLineEdit *editor = static_cast<QLineEdit *>(rawEditor);

    DAVA::VariantType variant = index.data(DAVA::VariantTypeEditRole).value<DAVA::VariantType>();
    editor->setText(QString("%1").arg(variant.AsFloat()));

    PropertyAbstractEditor::SetValueModified(editor, false);
}

void ItemDelegateForFloat::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    QLineEdit *editor = static_cast<QLineEdit *>(rawEditor);
    if (!PropertyAbstractEditor::IsValueModified(editor))
        return;

    QVariant variant;
    variant.setValue<DAVA::VariantType>(DAVA::VariantType(editor->text().toFloat()));

    model->setData(index, variant, DAVA::VariantTypeEditRole);
}

void ItemDelegateForFloat::OnValueChanged()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->NeedCommitData(editor);
}
