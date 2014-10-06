#include "ItemDelegateForPropertyEnum.h"
#include <QComboBox>
#include <QLayout>
#include "UIControls/BaseProperty.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "PropertiesTreeModel.h"

ItemDelegateForPropertyEnum::ItemDelegateForPropertyEnum(PropertiesTreeItemDelegate *delegate)
    : PropertyAbstractEditor(delegate)
{
}

ItemDelegateForPropertyEnum::~ItemDelegateForPropertyEnum()
{
}

QWidget * ItemDelegateForPropertyEnum::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->setObjectName(QString::fromUtf8("comboBox"));
    connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged()));

    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    const EnumMap *enumMap = property->GetEnumMap();
    DVASSERT(enumMap);

    comboBox->blockSignals(true);
    for (size_t i = 0; i < enumMap->GetCount(); ++i)
    {
        int value = 0;
        if (enumMap->GetValue(i, value))
        {
            QVariant variantValue;
            variantValue.setValue<DAVA::VariantType>(DAVA::VariantType(value));
            comboBox->addItem(QString(enumMap->ToString(value)),variantValue);
        }
    }
    comboBox->blockSignals(false);

    return comboBox;
}

void ItemDelegateForPropertyEnum::setEditorData( QWidget * editor, const QModelIndex & index ) const 
{
    QComboBox *comboBox = editor->findChild<QComboBox *>("comboBox");

    editor->blockSignals(true);
    int comboIndex = comboBox->findText(index.data(Qt::DisplayRole).toString());
    comboBox->setCurrentIndex(comboIndex);
    editor->blockSignals(false);

    PropertyAbstractEditor::SetValueModified(editor, false);
}

bool ItemDelegateForPropertyEnum::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (PropertyAbstractEditor::setModelData(editor, model, index))
        return true;

    QComboBox *comboBox = editor->findChild<QComboBox *>("comboBox");

    return model->setData(index, comboBox->itemData(comboBox->currentIndex()), Qt::EditRole);
}

void ItemDelegateForPropertyEnum::OnCurrentIndexChanged()
{
    QWidget *comboBox = qobject_cast<QWidget *>(sender());
    if (!comboBox)
        return;

    QWidget *editor = comboBox->parentWidget();
    if (!editor)
        return;

    PropertyAbstractEditor::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}

