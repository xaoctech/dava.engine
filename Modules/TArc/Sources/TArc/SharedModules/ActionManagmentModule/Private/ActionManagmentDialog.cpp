#include "TArc/SharedModules/ActionManagmentModule/Private/ActionManagmentDialog.h"
#include "TArc/SharedModules/ActionManagmentModule/Private/ShortcutsModel.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/ReflectedButton.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Base/GlobalEnum.h>

#include <QAction>
#include <QInputDialog>
#include <QTreeView>
#include <QGridLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QKeyEvent>

ENUM_DECLARE(Qt::ShortcutContext)
{
    ENUM_ADD_DESCR(Qt::WidgetWithChildrenShortcut, "Widget");
    ENUM_ADD_DESCR(Qt::WindowShortcut, "Window");
    ENUM_ADD_DESCR(Qt::ApplicationShortcut, "Application");
}

namespace DAVA
{
namespace TArc
{
ActionManagmentDialog::ActionManagmentDialog(ContextAccessor* accessor_, UIManager* ui_)
    : accessor(accessor_)
    , ui(ui_)
{
    Reflection model = Reflection::Create(ReflectedObject(this));
    QtVBoxLayout* layout = new QtVBoxLayout(this);

    QtHBoxLayout* schemesLayout = new QtHBoxLayout();
    {
        ComboBox::Params params(accessor, ui, mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "currentScheme";
        params.fields[ComboBox::Fields::Enumerator] = "schemes";
        schemesLayout->AddControl(new ComboBox(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "addIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "addToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "addScheme";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "removeIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "removeToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "removeScheme";
        params.fields[ReflectedButton::Fields::Enabled] = "removeButtonEnabled";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "importIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "importToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "importScheme";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    {
        ReflectedButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedButton::Fields::Icon] = "exportIcon";
        params.fields[ReflectedButton::Fields::IconSize] = "iconSize";
        params.fields[ReflectedButton::Fields::Tooltip] = "exportToolTip";
        params.fields[ReflectedButton::Fields::AutoRaise] = "autoRaise";
        params.fields[ReflectedButton::Fields::Clicked] = "exportScheme";
        schemesLayout->AddControl(new ReflectedButton(params, accessor, model, this));
    }
    layout->addLayout(schemesLayout);

    treeView = new QTreeView(this);
    shortcutsModel = new ShortcutsModel();
    treeView->setModel(shortcutsModel);
    layout->addWidget(treeView);

    QGridLayout* currentSequencesLayout = new QGridLayout();
    {
        ComboBox::Params params(accessor, ui, mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "currentSequence";
        params.fields[ComboBox::Fields::Enumerator] = "sequences";
        params.fields[ComboBox::Fields::MultipleValueText] = "emptySequenceText";
        params.fields[ComboBox::Fields::IsReadOnly] = "sequencesReadOnly";
        ComboBox* comboBox = new ComboBox(params, accessor, model, this);
        currentSequencesLayout->addWidget(comboBox->ToWidgetCast(), 0, 0, 1, 2);
    }
    {
        ReflectedPushButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedPushButton::Fields::Clicked] = "removeSequence";
        params.fields[ReflectedPushButton::Fields::Text] = "removeSequenceText";
        params.fields[ReflectedPushButton::Fields::Enabled] = "removeSequenceEnabled";
        ReflectedPushButton* button = new ReflectedPushButton(params, accessor, model, this);
        currentSequencesLayout->addWidget(button->ToWidgetCast(), 0, 2, 1, 1);
    }
    {
        ComboBox::Params params(accessor, ui, mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "currentContext";
        params.fields[ComboBox::Fields::IsReadOnly] = "contextReadOnly";
        ComboBox* comboBox = new ComboBox(params, accessor, model, this);
        currentSequencesLayout->addWidget(comboBox->ToWidgetCast(), 1, 0, 1, 1);
    }
    {
        LineEdit::Params params(accessor, ui, mainWindowKey);
        params.fields[LineEdit::Fields::Text] = "shortcutText";
        params.fields[LineEdit::Fields::IsEnabled] = "shortcutEnabled";
        LineEdit* lineEdit = new LineEdit(params, accessor, model, this);
        lineEdit->ToWidgetCast()->installEventFilter(this);
        currentSequencesLayout->addWidget(lineEdit->ToWidgetCast(), 1, 1, 1, 1);
    }
    {
        ReflectedPushButton::Params params(accessor, ui, mainWindowKey);
        params.fields[ReflectedPushButton::Fields::Clicked] = "assignShortcut";
        params.fields[ReflectedPushButton::Fields::Text] = "assignButtonText";
        params.fields[ReflectedPushButton::Fields::Enabled] = "assignButtonEnabled";
        ReflectedPushButton* button = new ReflectedPushButton(params, accessor, model, this);
        currentSequencesLayout->addWidget(button->ToWidgetCast(), 1, 2, 1, 1);
    }
    layout->addLayout(currentSequencesLayout);

    QItemSelectionModel* selectionModel = new QItemSelectionModel(shortcutsModel);
    treeView->setSelectionModel(selectionModel);
    connections.AddConnection(selectionModel, &QItemSelectionModel::selectionChanged, MakeFunction(this, &ActionManagmentDialog::OnActionSelected));

    UpdateSchemes();
    treeView->expandAll();
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->resizeColumnToContents(0);
}

bool ActionManagmentDialog::eventFilter(QObject* obj, QEvent* e)
{
    QEvent::Type t = e->type();
    bool processed = false;
    switch (t)
    {
    case QEvent::KeyPress:
    {
        int key = 0;
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        if (modifiers.testFlag(Qt::ControlModifier))
        {
            key += Qt::CTRL;
        }
        if (modifiers.testFlag(Qt::AltModifier))
        {
            key += Qt::ALT;
        }
        if (modifiers.testFlag(Qt::MetaModifier))
        {
            key += Qt::META;
        }
        if (modifiers.testFlag(Qt::ShiftModifier))
        {
            key += Qt::SHIFT;
        }

        int inputKey = keyEvent->key();
        if (inputKey != Qt::Key_Control &&
            inputKey != Qt::Key_Shift &&
            inputKey != Qt::Key_Alt &&
            inputKey != Qt::Key_Meta)
        {
            key += inputKey;
        }

        shortcutText = QKeySequence(key);
        processed = true;
    }
    break;
    default:
        break;
    }

    if (processed == true)
    {
        return true;
    }

    return QDialog::eventFilter(obj, e);
}

String ActionManagmentDialog::GetCurrentScheme() const
{
    return ui->GetCurrentScheme();
}

void ActionManagmentDialog::SetCurrentScheme(const String& scheme)
{
    ui->SetCurrentScheme(scheme);
    UpdateSchemes();
}

void ActionManagmentDialog::AddScheme()
{
    QInputDialog dlg;
    dlg.setObjectName("newSchemeNameDlg");
    dlg.setInputMode(QInputDialog::TextInput);

    QString newSchemeName;
    while (true)
    {
        int result = ui->ShowModalDialog(mainWindowKey, &dlg);
        if (result == QDialog::Rejected)
            break;

        QString inputValue = dlg.textValue();
        String value = inputValue.toStdString();
        auto iter = std::find(schemes.begin(), schemes.end(), value);
        if (iter == schemes.end())
        {
            newSchemeName = inputValue;
            break;
        }

        NotificationParams notifParams;
        notifParams.title = "Add key binding scheme";
        notifParams.message.type = Result::RESULT_ERROR;
        notifParams.message.message = Format("Key binding scheme with name \"%s\" already exists", value.c_str());
        ui->ShowNotification(mainWindowKey, notifParams);
    }

    if (newSchemeName.isEmpty() == false)
    {
        String schemeToAdd = newSchemeName.toStdString();
        ui->AddScheme(schemeToAdd);
        ui->SetCurrentScheme(schemeToAdd);
        UpdateSchemes();
    }
}

void ActionManagmentDialog::RemoveScheme()
{
    ui->RemoveScheme(ui->GetCurrentScheme());
    UpdateSchemes();
}

void ActionManagmentDialog::ImportScheme()
{
    FileDialogParams params;
    params.title = "Import key bindings scheme";
    params.filters = "Key binding scheme (*.kbs)";
    QString fileName = ui->GetOpenFileName(mainWindowKey, params);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    String schemeName = ui->ImportScheme(FilePath(fileName.toStdString()));
    ui->SetCurrentScheme(schemeName);
    UpdateSchemes();
}

void ActionManagmentDialog::ExportScheme()
{
    FileDialogParams params;
    params.title = "Import key bindings scheme";
    params.filters = "Key binding scheme (*.kbs)";
    QString fileName = ui->GetSaveFileName(mainWindowKey, params);
    if (fileName.isEmpty() == true)
    {
        return;
    }

    ui->ExportScheme(FilePath(fileName.toStdString()), ui->GetCurrentScheme());
}

void ActionManagmentDialog::UpdateSchemes()
{
    Vector<QString> expandedBlocks;
    int rows = shortcutsModel->rowCount(QModelIndex());
    for (int i = 0; i < rows; ++i)
    {
        QModelIndex index = shortcutsModel->index(i, 0, QModelIndex());
        if (treeView->isExpanded(index))
        {
            expandedBlocks.push_back(index.data(Qt::DisplayRole).value<QString>());
        }
    }

    Vector<String> s = ui->GetKeyBindingsSchemeNames();
    schemes = Set<String>(s.begin(), s.end());
    shortcutsModel->SetData(ui->GetKeyBindableActions());

    for (QString expandedBlock : expandedBlocks)
    {
        QModelIndex blockIndex = shortcutsModel->GetIndex(expandedBlock);
        treeView->expand(blockIndex);
    }

    QModelIndex selectedIndex = shortcutsModel->GetIndex(selectedBlockName, selectedAction);
    treeView->selectionModel()->select(selectedIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void ActionManagmentDialog::RemoveSequence()
{
    QKeySequence sequence = QKeySequence::fromString(QString::fromStdString(currentSequence), QKeySequence::NativeText);
    ui->RemoveShortcut(sequence, selectedAction);
    UpdateSchemes();
}

bool ActionManagmentDialog::CanBeAssigned() const
{
    if (isSelectedActionReadOnly == true)
    {
        return false;
    }

    bool modifFound = false;
    bool keyFound = false;
    for (int i = 0; i < shortcutText.count(); ++i)
    {
        int key = shortcutText[i];
        if ((key & Qt::CTRL) == Qt::CTRL ||
            (key & Qt::ALT) == Qt::ALT ||
            (key & Qt::SHIFT) == Qt::SHIFT ||
            (key & Qt::META) == Qt::META)
        {
            modifFound = true;
        }

        key = key & (~(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::META));
        if (key != 0)
        {
            keyFound = true;
        }
    }
    return modifFound == true && keyFound == true;
}

String ActionManagmentDialog::GetShortcutText() const
{
    return shortcutText.toString(QKeySequence::NativeText).toStdString();
}

void ActionManagmentDialog::SetShortcutText(const String&)
{
}

void ActionManagmentDialog::AssignShortcut()
{
    ui->AddShortcut(shortcutText, selectedAction);
    shortcutText = QKeySequence();
    UpdateSchemes();
}

Qt::ShortcutContext ActionManagmentDialog::GetContext() const
{
    return context;
}

void ActionManagmentDialog::SetContext(Qt::ShortcutContext v)
{
    context = v;
    if (selectedAction != nullptr)
    {
        ui->SetActionContext(selectedAction, context);
        UpdateSchemes();
    }
}

void ActionManagmentDialog::OnActionSelected(const QItemSelection& selected, const QItemSelection&)
{
    QModelIndexList indexes = selected.indexes();
    const KeyBindableAction* action = nullptr;
    if (indexes.isEmpty() == false)
    {
        action = shortcutsModel->GetKeyBindableAction(indexes.front());
    }

    selectedAction = nullptr;
    currentSequence = String();
    selectedBlockName = QString();
    isSelectedActionReadOnly = false;
    sequences.clear();

    if (action != nullptr)
    {
        foreach (const QKeySequence& seq, action->sequences)
        {
            sequences.insert(seq.toString(QKeySequence::NativeText).toStdString());
        }
        if (sequences.empty() == false)
        {
            currentSequence = *sequences.begin();
        }
        selectedBlockName = action->blockName;
        context = action->context;
        selectedAction = action->action;
        isSelectedActionReadOnly = action->isReadOnly;
    }
}

DAVA_REFLECTION_IMPL(ActionManagmentDialog)
{
    ReflectionRegistrator<ActionManagmentDialog>::Begin()
    // schemes combobox
    .Field("schemes", &ActionManagmentDialog::schemes)
    .Field("currentScheme", &ActionManagmentDialog::GetCurrentScheme, &ActionManagmentDialog::SetCurrentScheme)
    // add, remove, import, export buttons
    .Field("iconSize", [](ActionManagmentDialog*) { return QSize(16, 16); }, nullptr)
    .Field("autoRaise", [](ActionManagmentDialog*) { return false; }, nullptr)
    .Field("addIcon", [](ActionManagmentDialog*) { return QIcon(":/TArc/Resources/cplus.png"); }, nullptr)
    .Field("addToolTip", [](ActionManagmentDialog*) { return "Create new key binding scheme"; }, nullptr)
    .Field("removeIcon", [](ActionManagmentDialog*) { return QIcon(":/TArc/Resources/cminus.png"); }, nullptr)
    .Field("removeToolTip", [](ActionManagmentDialog*) { return "Delete current key binding scheme"; }, nullptr)
    .Field("removeButtonEnabled", [](ActionManagmentDialog* obj) { return obj->schemes.size() > 1; }, nullptr)
    .Field("importIcon", [](ActionManagmentDialog*) { return QIcon(":/TArc/Resources/import.png"); }, nullptr)
    .Field("importToolTip", [](ActionManagmentDialog*) { return "Import key binding scheme"; }, nullptr)
    .Field("exportIcon", [](ActionManagmentDialog*) { return QIcon(":/TArc/Resources/export.png"); }, nullptr)
    .Field("exportToolTip", [](ActionManagmentDialog*) { return "Export current key binding scheme"; }, nullptr)
    .Method("addScheme", &ActionManagmentDialog::AddScheme)
    .Method("removeScheme", &ActionManagmentDialog::RemoveScheme)
    .Method("importScheme", &ActionManagmentDialog::ImportScheme)
    .Method("exportScheme", &ActionManagmentDialog::ExportScheme)
    // shortcuts combobox
    .Field("currentSequence", &ActionManagmentDialog::currentSequence)
    .Field("sequences", &ActionManagmentDialog::sequences)
    .Field("emptySequenceText", [](ActionManagmentDialog* obj) { return ""; }, nullptr)
    .Field("sequencesReadOnly", [](ActionManagmentDialog* obj) { return obj->sequences.empty() == true; }, nullptr)
    // remove shortcut
    .Field("removeSequenceEnabled", [](ActionManagmentDialog* obj) { return obj->currentSequence.empty() == false && obj->isSelectedActionReadOnly == false; }, nullptr)
    .Field("removeSequenceText", [](ActionManagmentDialog*) { return "Remove"; }, nullptr)
    .Method("removeSequence", &ActionManagmentDialog::RemoveSequence)
    // context combobox
    .Field("currentContext", &ActionManagmentDialog::GetContext, &ActionManagmentDialog::SetContext)[M::EnumT<Qt::ShortcutContext>()]
    .Field("contextReadOnly", [](ActionManagmentDialog* obj) { return obj->selectedAction == nullptr || obj->isSelectedActionReadOnly == true; }, nullptr)
    //  Shortcut line edit
    .Field("shortcutText", &ActionManagmentDialog::GetShortcutText, &ActionManagmentDialog::SetShortcutText)
    .Field("shortcutEnabled", [](ActionManagmentDialog* obj) { return obj->selectedAction != nullptr && obj->isSelectedActionReadOnly == false; }, nullptr)
    // assign button
    .Field("assignButtonText", [](ActionManagmentDialog*) { return "Assign"; }, nullptr)
    .Field("assignButtonEnabled", &ActionManagmentDialog::CanBeAssigned, nullptr)
    .Method("assignShortcut", &ActionManagmentDialog::AssignShortcut)
    .End();
}

} // namespace TArc
} // namespace DAVA