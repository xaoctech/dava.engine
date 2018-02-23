#include "VisualScriptEditor/Private/VisualScriptEditorPropertiesView.h"
#include "VisualScriptEditor/Private/VisualScriptEditorData.h"

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>
#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/PropertyPanel/TimerUpdater.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Controls/Widget.h>
#include <TArc/Controls/CommonStrings.h>
#include <TArc/Utils/ReflectionHelpers.h>
#include <TArc/Utils/QtDelayedExecutor.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Utils/Utils.h>
#include <TArc/DataProcessing/Common.h>


#include <VisualScript/VisualScript.h>

#include <QLabel>
#include <QGridLayout>

namespace DAVA
{
namespace VisualScriptEditorPropertiesViewDetails
{
enum eValuePropertyType
{
    AddValueProperty = PropertyNode::DomainSpecificProperty
};

const char* chooseValueTypeString = "Choose value type for adding";
class AddValueWidget : public QWidget
{
public:
    AddValueWidget(ContextAccessor* accessor, UI* ui, const WindowKey& wndKey)
    {
        VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
        if (editorData->activeDescriptor != nullptr)
        {
            script = editorData->activeDescriptor->script;
        }
        DVASSERT(script != nullptr);

        { //prepare DATA
            typesEnumeration.push_back(FastName(chooseValueTypeString));
            for (const std::pair<FastName, Any>& pair : script->defaultValues)
            {
                typesEnumeration.push_back(pair.first);
            }
        }

        { //prepare UI
            using namespace DAVA;

            QGridLayout* layout = new QGridLayout(this);
            layout->setMargin(5);
            layout->setSpacing(3);

            layout->addWidget(new QLabel("Type:", this), 0, 0, 1, 1);
            layout->addWidget(new QLabel("Key:", this), 1, 0, 1, 1);

            Reflection r = Reflection::Create(ReflectedObject(this));
            {
                ComboBox::Params params(accessor, ui, wndKey);
                params.fields[ComboBox::Fields::Enumerator] = "types";
                params.fields[ComboBox::Fields::Value] = "currentType";
                ComboBox* typesCombo = new ComboBox(params, accessor, r, this);
                layout->addWidget(typesCombo->ToWidgetCast(), 0, 1, 1, 2);
            }

            {
                LineEdit::Params params(accessor, ui, wndKey);
                params.fields[LineEdit::Fields::Text] = "key";
                params.fields[LineEdit::Fields::PlaceHolder] = "keyHint";
                params.fields[LineEdit::Fields::ImmediateText] = "keyImmediateText";

                LineEdit* keyEdit = new LineEdit(params, accessor, r, this);
                keyEdit->SetObjectName(QString("keyEdit"));
                lineEdit = keyEdit->ToWidgetCast();
                layout->addWidget(lineEdit, 1, 1, 1, 2);
                setFocusProxy(lineEdit);
            }

            {
                ReflectedPushButton::Params params(accessor, ui, wndKey);
                params.fields[ReflectedPushButton::Fields::Clicked] = "addValue";
                params.fields[ReflectedPushButton::Fields::Text] = "addValueText";
                params.fields[ReflectedPushButton::Fields::Enabled] = "addEnabled";
                params.fields[ReflectedPushButton::Fields::Tooltip] = "tooltip";
                ReflectedPushButton* button = new ReflectedPushButton(params, accessor, r, this);
                layout->addWidget(button->ToWidgetCast(), 3, 1, 1, 2);
            }

            setAttribute(Qt::WA_DeleteOnClose);

            setWindowFlags(static_cast<Qt::WindowFlags>(Qt::FramelessWindowHint | Qt::Popup));
            setWindowOpacity(0.95);
        }
    }

    void Show()
    {
        show();
        lineEdit->setFocus();
    }

    bool CanAddValue() const
    {
        if (currentTypeIndex == 0)
        {
            return false;
        }

        if (keyImmediateValue.IsValid() == false || keyImmediateValue.empty() == true || keyImmediateValue == FastName(""))
        {
            return false;
        }

        //TODO: use immediate value from Input
        return script->dataRegistry.count(keyImmediateValue) == 0;
    }

    QString GetKeyHint() const
    {
        return QString("Enter key");
    }

    QString GetAddButtonHint() const
    {
        if (currentTypeIndex == 0)
        {
            return QString("Select type");
        }

        if (key.empty() == true || key.IsValid() == false)
        {
            return QString("Enter valid key");
        }

        if (script->dataRegistry.count(key) != 0)
        {
            return QString("Enter unique key");
        }

        return QString("Add Value");
    }

    void SetKeyImmediateText(const String& immText)
    {
        keyImmediateValue = FastName(immText);
    }

private:
    void AddValue()
    {
        if (script->dataRegistry.count(key) == 0)
        {
            DVASSERT(script->defaultValues.count(typesEnumeration[currentTypeIndex]) != 0);

            Any value = script->defaultValues.at(typesEnumeration[currentTypeIndex]);
            script->dataRegistry[key] = value;

            close();
        }
    }

    FastName key = FastName("");
    FastName keyImmediateValue = FastName("");

    size_t currentTypeIndex = 0;
    Vector<FastName> typesEnumeration;

    QWidget* lineEdit = nullptr;
    VisualScript* script = nullptr;

    DAVA_REFLECTION(AddValueWidget);
};

DAVA_REFLECTION_IMPL(AddValueWidget)
{
    DAVA::ReflectionRegistrator<AddValueWidget>::Begin()
    //key
    .Field("key", &AddValueWidget::key)
    .Field("keyHint", &AddValueWidget::GetKeyHint, nullptr)
    .Method("keyImmediateText", &AddValueWidget::SetKeyImmediateText)
    //combo
    .Field("currentType", &AddValueWidget::currentTypeIndex)
    .Field("types", &AddValueWidget::typesEnumeration)
    // button add
    .Field("addValueText", []() { return QString("Add Value"); }, nullptr)
    .Field("tooltip", &AddValueWidget::GetAddButtonHint, nullptr)
    .Field("addEnabled", &AddValueWidget::CanAddValue, nullptr)
    .Method("addValue", &AddValueWidget::AddValue)
    //end
    .End();
}

class ValueCreatorComponentValue : public BaseComponentValue
{
public:
    ValueCreatorComponentValue()
    {
    }

    ~ValueCreatorComponentValue()
    {
        if (widget.isNull() == false)
        {
            widget->deleteLater();
        }
    }

    bool IsSpannedControl() const override
    {
        return true;
    }

protected:
    Any GetMultipleValue() const override
    {
        return Any();
    }

    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override
    {
        return false;
    }

    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
        ValueCreatorComponentValue* nonConstThis = const_cast<ValueCreatorComponentValue*>(this);
        ReflectedPushButton::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ReflectedPushButton::Fields::Clicked] = "addValue";
        params.fields[ReflectedPushButton::Fields::Text] = "text";
        params.fields[ReflectedPushButton::Fields::Enabled] = "isButtonEnabled";

        ReflectedPushButton* button = new ReflectedPushButton(params, GetAccessor(), model, parent);
        return button;
    }

private:
    void AddValue()
    {
        if (widget == nullptr)
        {
            widget = new AddValueWidget(GetAccessor(), GetUI(), GetWindowKey());
        }

        widget->Show();

        QWidget* thisWidget = editorWidget->ToWidgetCast();
        QPoint topLeft = thisWidget->mapToGlobal(QPoint(0, 0));

        QRect wRect = widget->geometry();
        QPoint wPos = QPoint(topLeft.x() - wRect.width(), topLeft.y());

        widget->move(wPos);
    }

    bool IsButtonEnabled() const
    {
        VisualScriptEditorData* editorData = GetAccessor()->GetGlobalContext()->GetData<VisualScriptEditorData>();
        return (editorData->activeDescriptor != nullptr);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ValueCreatorComponentValue, BaseComponentValue)
    {
        ReflectionRegistrator<ValueCreatorComponentValue>::Begin()
        .Field("text", []() { return QString("Add Value"); }, nullptr)
        .Method("addValue", &ValueCreatorComponentValue::AddValue)
        .Field("isButtonEnabled", &ValueCreatorComponentValue::IsButtonEnabled, nullptr)
        .End();
    }

    QPointer<AddValueWidget> widget;
};

class EditorComponentExtensionExt : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override
    {
        if (node->propertyType == eValuePropertyType::AddValueProperty)
        {
            using namespace DAVA;

            std::unique_ptr<BaseComponentValue> editor = std::make_unique<ValueCreatorComponentValue>();
            BaseComponentValue::Style style;
            style.fontBold = true;
            style.fontItalic = true;
            style.fontColor = QPalette::ButtonText;
            style.bgColor = QPalette::AlternateBase;
            editor->SetStyle(style);
            return std::move(editor);
        }
        return EditorComponentExtension::GetEditor(node);
    }
};

class ChildCreatorExtensionExt : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const override
    {
        using namespace DAVA;

        if (parent->propertyType == PropertyNode::SelfRoot)
        {
            VisualScriptEditorReflectionHolder* holder = parent->field.ref.GetValue().Get<VisualScriptEditorReflectionHolder*>(nullptr);
            for (const Reflection& ref : holder->reflectedModels)
            {
                Reflection::Field f;

                const ReflectedType* refType = GetValueReflectedType(ref);
                f.key = refType->GetPermanentName();
                f.ref = ref;

                children.push_back(allocator->CreatePropertyNode(parent, std::move(f), static_cast<int32>(children.size()), PropertyNode::RealProperty));
            }

            Reflection::Field addValueField;
            addValueField.key = "Add Value";
            addValueField.ref = parent->field.ref;
            std::shared_ptr<PropertyNode> addValueNode = allocator->CreatePropertyNode(parent, std::move(addValueField), PropertyNode::InvalidSortKey - 1, eValuePropertyType::AddValueProperty);
            children.push_back(addValueNode);
        }
        else
        {
            ChildCreatorExtension::ExposeChildren(parent, children);
        }
    }
};

class ModifyExtensionExt : public ModifyExtension
{
public:
    ModifyExtensionExt(std::weak_ptr<PropertiesView::Updater> updater_, ContextAccessor* accessor_, UI* ui_)
        : updater(updater_)
        , accessor(accessor_)
        , ui(ui_)
    {
    }

    void BeginBatch(const String& text, uint32 commandCount) override
    {
    }
    void ProduceCommand(const std::shared_ptr<PropertyNode>& node, const Any& newValue) override
    {
        ProduceCommand(node->field, newValue);
    }

    void ProduceCommand(const Reflection::Field& object, const Any& newValue) override
    {
        object.ref.SetValueWithCast(newValue);
        UpdateView();
    }

    void RemoveField(const Reflection& collection, const Any& key) override
    {
        FastName fnKey = key.Get<FastName>(FastName());

        VisualScriptEditorData* editorData = accessor->GetGlobalContext()->GetData<VisualScriptEditorData>();
        DVASSERT(editorData->activeDescriptor != nullptr);

        VisualScript* script = editorData->activeDescriptor->script;
        const Vector<VisualScriptNode*>& scriptNodes = script->GetNodes();
        for (const VisualScriptNode* node : scriptNodes)
        {
            const VisualScriptGetVarNode* getNode = dynamic_cast<const VisualScriptGetVarNode*>(node);
            const VisualScriptSetVarNode* setNode = dynamic_cast<const VisualScriptSetVarNode*>(node);
            if (((getNode != nullptr) && (getNode->GetVarPath() == fnKey))
                || ((setNode != nullptr) && (setNode->GetVarPath() == fnKey)))
            {
                NotificationParams notifParams;
                notifParams.title = "Error";
                notifParams.message.message = Format("Can't remove %s. There are Var nodes created from %s", fnKey.c_str(), fnKey.c_str());
                ui->ShowNotification(mainWindowKey, notifParams);
                return;
            }
        }

        collection.RemoveField(key);
        UpdateView();
    }

    void Exec(std::unique_ptr<Command>&& command) override
    {
        command->Redo();
        UpdateView();
    }

    void EndBatch() override
    {
    }

private:
    void UpdateView()
    {
        executor.DelayedExecute([this]() {
            std::shared_ptr<PropertiesView::Updater> u = updater.lock();
            if (u != nullptr)
            {
                u->update.Emit(PropertiesView::FullUpdate);
            }
        });
    }

    std::weak_ptr<PropertiesView::Updater> updater;
    QtDelayedExecutor executor;

    ContextAccessor* accessor = nullptr;
    UI* ui = nullptr;
};

} //VisualScriptEditorPropertiesViewDetails

PropertiesView* VisualScriptEditorPropertiesView::CreateProperties(ContextAccessor* accessor, UI* ui, std::shared_ptr<PropertiesView::Updater> updater)
{
    PropertiesView::Params propertiesViewParams(mainWindowKey);
    propertiesViewParams.accessor = accessor;
    propertiesViewParams.isInDevMode = false;
    propertiesViewParams.showToolBar = false;
    propertiesViewParams.isDragAccepted = true;
    propertiesViewParams.ui = ui;
    propertiesViewParams.objectsField.type = ReflectedTypeDB::Get<VisualScriptEditorData>();
    propertiesViewParams.objectsField.fieldName = FastName(VisualScriptEditorData::reflectionHolderProperty);
    propertiesViewParams.settingsNodeName = "VisualScriptEditorDialogPropertyPanel";
    propertiesViewParams.updater = updater;

    PropertiesView* propertiesView = new PropertiesView(propertiesViewParams);
    propertiesView->RegisterExtension(std::make_shared<VisualScriptEditorPropertiesViewDetails::ChildCreatorExtensionExt>());
    propertiesView->RegisterExtension(std::make_shared<VisualScriptEditorPropertiesViewDetails::EditorComponentExtensionExt>());
    propertiesView->RegisterExtension(std::make_shared<VisualScriptEditorPropertiesViewDetails::ModifyExtensionExt>(updater, accessor, ui));

    return propertiesView;
}

} // DAVA
