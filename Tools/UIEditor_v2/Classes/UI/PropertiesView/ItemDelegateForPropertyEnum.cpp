#include "ItemDelegateForPropertyEnum.h"
#include <QComboBox>
#include "UIControls/BaseProperty.h"
#include "PropertiesTreeItemDelegate.h"

ItemDelegateForPropertyEnum::ItemDelegateForPropertyEnum(PropertiesTreeItemDelegate *delegate)
    : PropertyAbstractEditor()
    , itemDelegate(delegate)
{
}

ItemDelegateForPropertyEnum::~ItemDelegateForPropertyEnum()
{
}

QWidget * ItemDelegateForPropertyEnum::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const 
{
    QComboBox *editor = new QComboBox(parent);
    connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged()));

    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    const EnumMap *enumMap = property->GetEnumMap();
    DVASSERT(enumMap);

    editor->blockSignals(true);
    for (size_t i = 0; i < enumMap->GetCount(); ++i)
    {
        int value = 0;
        if (enumMap->GetValue(i, value))
        {
            editor->addItem(QString(enumMap->ToString(value)), QVariant(value));
        }
    }
    editor->blockSignals(false);

    return editor;
}

void ItemDelegateForPropertyEnum::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());

    editor->blockSignals(true);
    comboBox->setCurrentIndex(comboBox->findData(QVariant(property->GetValue().AsInt32())));
    editor->blockSignals(false);
}

void ItemDelegateForPropertyEnum::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!PropertyAbstractEditor::IsValueModified(comboBox))
        return;

    int value = comboBox->itemData(comboBox->currentIndex()).toInt();
    model->setData(index, value);
}

void ItemDelegateForPropertyEnum::OnCurrentIndexChanged()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->NeedCommitData(editor);
}

