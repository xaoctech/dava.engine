#include "ItemDelegateForFloat.h"
#include "FileSystem/VariantType.h"
#include "PropertiesTreeModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include <QLineEdit>
#include <QDoubleValidator>
#include <QLayout>


ItemDelegateForFloat::ItemDelegateForFloat( PropertiesTreeItemDelegate *delegate )
    : PropertyAbstractEditor(delegate)
{

}

ItemDelegateForFloat::~ItemDelegateForFloat()
{

}

QWidget * ItemDelegateForFloat::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(OnValueChanged()));
    lineEdit->setValidator( new QDoubleValidator(-999999.0, 999999.0, 6, lineEdit) );

    return lineEdit;
}

void ItemDelegateForFloat::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    editor->blockSignals(true);
    editor->setText(QString("%1").arg(variant.AsFloat()));
    editor->blockSignals(false);

    PropertyAbstractEditor::SetValueModified(editor, false);
}

bool ItemDelegateForFloat::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (PropertyAbstractEditor::setModelData(rawEditor, model, index))
        return true;

    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    QVariant variant;
    variant.setValue<DAVA::VariantType>(DAVA::VariantType(editor->text().toFloat()));

    return model->setData(index, variant, Qt::EditRole);
}

void ItemDelegateForFloat::OnValueChanged()
{
    QWidget *lineEdit = qobject_cast<QWidget *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget();
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
