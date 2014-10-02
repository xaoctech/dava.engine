#include "ItemDelegateForPropertyEnum.h"
#include <QComboBox>
#include "UIControls/BaseProperty.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesTreeModel.h"

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
            QVariant variantValue;
            variantValue.setValue<DAVA::VariantType>(DAVA::VariantType(value));
            editor->addItem(QString(enumMap->ToString(value)),variantValue);
        }
    }
    editor->blockSignals(false);

    return editor;
}

void ItemDelegateForPropertyEnum::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);

    editor->blockSignals(true);
    int comboIndex = comboBox->findText(index.data(Qt::DisplayRole).toString());
    comboBox->setCurrentIndex(comboIndex);
    editor->blockSignals(false);

    PropertyAbstractEditor::SetValueModified(editor, false);
}

void ItemDelegateForPropertyEnum::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    if (!PropertyAbstractEditor::IsValueModified(comboBox))
        return;

    model->setData(index, comboBox->itemData(comboBox->currentIndex()), DAVA::VariantTypeEditRole);
}

void ItemDelegateForPropertyEnum::OnCurrentIndexChanged()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->NeedCommitData(editor);
}

