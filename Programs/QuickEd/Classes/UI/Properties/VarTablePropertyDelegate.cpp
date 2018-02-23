#include "VarTablePropertyDelegate.h"

#include <DAVAEngine.h>
#include <QAction>
#include <QLineEdit>
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include "UI/Dialogs/VarTableEditorDialog.h"

#include <UI/Properties/VarTable.h>

using namespace DAVA;

VarTablePropertyDelegate::VarTablePropertyDelegate(PropertiesTreeItemDelegate* delegate)
    : BasePropertyDelegate(delegate)
{
}

VarTablePropertyDelegate::~VarTablePropertyDelegate()
{
}

QWidget* VarTablePropertyDelegate::createEditor(QWidget* parent, const PropertiesContext& context, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    QLineEdit* lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName("lineEdit");
    lineEdit->setReadOnly(true);
    connect(lineEdit, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
    return lineEdit;
}

void VarTablePropertyDelegate::setEditorData(QWidget* rawEditor, const QModelIndex& index) const
{
    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");
    editor->setProperty("modelIndex", index);

    QVariant variant = index.data(Qt::EditRole);
    editor->setProperty("modelData", variant);

    DAVA::Any any = variant.value<DAVA::Any>();
    QString stringValue;
    if (any.CanGet<DAVA::VarTable>())
    {
        VarTable varTable = any.Get<DAVA::VarTable>();
        stringValue = StringToQString("[" + varTable.GetNamesString() + "]");
    }
    else
    {
        DVASSERT(false);
    }
    editor->blockSignals(true);
    editor->setText(stringValue);
    editor->blockSignals(false);
}

bool VarTablePropertyDelegate::setModelData(QWidget* rawEditor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    QVariant variant;
    DAVA::Any value = index.data(Qt::EditRole).value<DAVA::Any>();
    if (value.CanGet<DAVA::VarTable>())
    {
        variant = editor->property("modelData");
        DAVA::Any any = variant.value<DAVA::Any>();
        VarTable varTable = any.Get<DAVA::VarTable>();
    }
    else
    {
        DVASSERT(false);
    }

    return model->setData(index, variant, Qt::EditRole);
}

void VarTablePropertyDelegate::enumEditorActions(QWidget* parent, const QModelIndex& index, QList<QAction*>& actions)
{
    editTableAction = new QAction(QIcon(":/Icons/configure.png"), tr("edit"), parent);
    editTableAction->setToolTip(tr("edit table"));
    actions.push_back(editTableAction);
    connect(editTableAction, &QAction::triggered, this, &VarTablePropertyDelegate::editTableClicked);

    BasePropertyDelegate::enumEditorActions(parent, index, actions);
}

void VarTablePropertyDelegate::editTableClicked()
{
    QAction* editPresetAction = qobject_cast<QAction*>(sender());
    if (!editPresetAction)
        return;

    QWidget* rawEditor = editPresetAction->parentWidget();
    if (!rawEditor)
        return;

    QLineEdit* editor = rawEditor->findChild<QLineEdit*>("lineEdit");
    QVariant variant = editor->property("modelData");
    DAVA::Any any = variant.value<DAVA::Any>();
    VarTable varTable = any.Get<DAVA::VarTable>();
    VarTableEditorDialog dialog(varTable, qApp->activeWindow());
    if (dialog.exec())
    {
        variant.setValue<Any>(dialog.GetValues());
        editor->setProperty("modelData", variant);
        BasePropertyDelegate::SetValueModified(rawEditor, true);
        itemDelegate->emitCommitData(rawEditor);
    }
}

void VarTablePropertyDelegate::OnEditingFinished()
{
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
    if (!lineEdit)
        return;

    QWidget* editor = lineEdit->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, lineEdit->isModified());
    itemDelegate->emitCommitData(editor);
}
