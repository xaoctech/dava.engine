#include "Classes/SlotSupportModule/Private/SlotComponentExtensions.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"
#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Commands2/AddComponentCommand.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/SlotSupportModule/Private/SlotTemplatesData.h"

#include <TArc/Controls/CommonStrings.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/FilePathEdit.h>
#include <TArc/Controls/Widget.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Controls/ListView.h>
#include <TArc/Controls/PopupLineEdit.h>
#include <TArc/Utils/ReflectionHelpers.h>
#include <TArc/Utils/Utils.h>
#include <TArc/Controls/PropertyPanel/PropertyPanelMeta.h>
#include <TArc/Controls/PropertyPanel/Private/TextComponentValue.h>

#include <Scene3D/Systems/SlotSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <FileSystem/FilePath.h>
#include <Base/BaseTypes.h>
#include <Base/Any.h>

#include <QHBoxLayout>

namespace PropertyPanel
{
class BaseSlotComponentValue : public DAVA::TArc::BaseComponentValue
{
protected:
    void ForEachSlotComponent(const DAVA::Function<bool(DAVA::SlotComponent*, bool)>& fn) const
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            std::shared_ptr<DAVA::TArc::PropertyNode> node = nodes[i];
            DAVA::SlotComponent* component = node->cachedValue.Cast<DAVA::SlotComponent*>(nullptr);
            DVASSERT(component != nullptr);
            if (fn(component, i == 0) == false)
            {
                break;
            }
        }
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(BaseSlotComponentValue, DAVA::TArc::BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<BaseSlotComponentValue>::Begin()
        .End();
    }
};

class SlotTypeFiltersComponentValue : public BaseSlotComponentValue
{
public:
    DAVA::Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const override
    {
        return false;
    }

    DAVA::TArc::ControlProxy* CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor) override
    {
        using namespace DAVA::TArc;

        Widget* w = new Widget(parent);
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0, 2, 2, 2);
        w->SetLayout(layout);

        ListView::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ListView::Fields::ValueList] = "filtersList";
        params.fields[ListView::Fields::CurrentValue] = "currentFilter";

        ListView* filtersControl = new ListView(params, wrappersProcessor, model, parent);
        filtersControl->ToWidgetCast()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        w->AddControl(filtersControl);

        Widget* buttonsBar = new Widget(w->ToWidgetCast());
        buttonsBar->SetLayout(new QVBoxLayout());
        w->AddControl(buttonsBar);

        {
            ReflectedButton::Params params(GetAccessor(), GetUI(), GetWindowKey());
            params.fields[ReflectedButton::Fields::AutoRaise] = "autoRise";
            params.fields[ReflectedButton::Fields::Icon] = "addButtonIcon";
            params.fields[ReflectedButton::Fields::Tooltip] = "addButtonTooltip";
            params.fields[ReflectedButton::Fields::Clicked] = "addTypeFilter";
            params.fields[ReflectedButton::Fields::Enabled] = "addButtonEnabled";
            ReflectedButton* addButton = new ReflectedButton(params, wrappersProcessor, model, w->ToWidgetCast());
            buttonsBar->AddControl(addButton);
        }

        {
            ReflectedButton::Params params(GetAccessor(), GetUI(), GetWindowKey());
            params.fields[ReflectedButton::Fields::AutoRaise] = "autoRise";
            params.fields[ReflectedButton::Fields::Icon] = "removeButtonIcon";
            params.fields[ReflectedButton::Fields::Tooltip] = "removeButtonTooltip";
            params.fields[ReflectedButton::Fields::Enabled] = "removeButtonEnabled";
            params.fields[ReflectedButton::Fields::Clicked] = "removeTypeFilter";
            ReflectedButton* addButton = new ReflectedButton(params, wrappersProcessor, model, w->ToWidgetCast());
            buttonsBar->AddControl(addButton);
        }

        return w;
    }

private:
    const DAVA::Set<DAVA::String>& GetTypeFilters() const
    {
        DAVA::Set<DAVA::FastName> intersection;
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst) {
            DAVA::Set<DAVA::FastName> currentIntersection;
            for (DAVA::uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
            {
                DAVA::FastName filter = component->GetTypeFilter(i);
                if (isFirst == true || intersection.count(filter) > 0)
                {
                    currentIntersection.insert(component->GetTypeFilter(i));
                }
            }

            std::swap(intersection, currentIntersection);
            return true;
        });

        filters.clear();
        std::transform(intersection.begin(), intersection.end(), std::inserter(filters, filters.end()), DAVA::Bind(&DAVA::FastName::c_str, DAVA::_1));
        return filters;
    }

    void AddTypeFilter()
    {
        using namespace DAVA::TArc;

        LineEdit::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[LineEdit::Fields::Text] = "addFilterPopupText";
        params.fields[LineEdit::Fields::PlaceHolder] = "filterEditPlaceholder";

        DAVA::Reflection popupModel = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
        DAVA::TArc::PopupLineEdit* popupLineEdit = new DAVA::TArc::PopupLineEdit(params, GetAccessor(), popupModel, realWidget);
        popupLineEdit->Show(realWidget->parentWidget()->mapToGlobal(realWidget->geometry().topLeft()));
    }

    void RemoveTypeFilter()
    {
        DAVA::FastName filterToRemove(currentFilter);
        DAVA::String descr = DAVA::Format("Remove type filter: %s", currentFilter.c_str());
        DAVA::TArc::ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(descr, static_cast<DAVA::uint32>(nodes.size()));
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst) {
            cmdInterface.Exec(std::make_unique<SlotTypeFilterEdit>(component, filterToRemove, false));
            return true;
        });

        ForceUpdate();
    }

    DAVA::String GetPopupText() const
    {
        return "";
    }

    void SetPopupText(const DAVA::String& filterName)
    {
        if (filterName.empty())
        {
            return;
        }

        DAVA::FastName filterToAdd(filterName);
        DAVA::String descr = DAVA::Format("Add type filter: %s", currentFilter.c_str());
        DAVA::TArc::ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(descr, static_cast<DAVA::uint32>(nodes.size()));
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst) {
            for (DAVA::uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
            {
                if (component->GetTypeFilter(i) == filterToAdd)
                {
                    return true;
                }
            }
            cmdInterface.Exec(std::make_unique<SlotTypeFilterEdit>(component, filterToAdd, true));
            return true;
        });
    }

    DAVA::Any GetCurrentFilter() const
    {
        if (currentFilter.empty())
        {
            return DAVA::Any();
        }
        return currentFilter;
    }

    void SetCurrentFilter(const DAVA::Any& v)
    {
        if (v.IsEmpty())
        {
            currentFilter = DAVA::String("");
        }
        else
        {
            currentFilter = v.Cast<DAVA::String>();
        }

        ForceUpdate();
    }

    mutable DAVA::Set<DAVA::String> filters;
    DAVA::String currentFilter = DAVA::String("");

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotTypeFiltersComponentValue, BaseSlotComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotTypeFiltersComponentValue>::Begin()
        .Field("filtersList", &SlotTypeFiltersComponentValue::GetTypeFilters, nullptr)
        .Field("currentFilter", &SlotTypeFiltersComponentValue::GetCurrentFilter, &SlotTypeFiltersComponentValue::SetCurrentFilter)
        .Field("autoRise", [](SlotTypeFiltersComponentValue*) { return false; }, nullptr)
        .Field("addButtonIcon", [](SlotTypeFiltersComponentValue*) { return DAVA::TArc::SharedIcon(":/QtIcons/cplus.png"); }, nullptr)
        .Field("addButtonTooltip", [](SlotTypeFiltersComponentValue*) { return "Add type filter"; }, nullptr)
        .Field("addButtonEnabled", [](SlotTypeFiltersComponentValue* v) { return v->filters.size() < DAVA::SlotComponent::MAX_FILTERS_COUNT; }, nullptr)
        .Method("addTypeFilter", &SlotTypeFiltersComponentValue::AddTypeFilter)
        .Field("removeButtonIcon", [](SlotTypeFiltersComponentValue*) { return DAVA::TArc::SharedIcon(":/QtIcons/cminus.png"); }, nullptr)
        .Field("removeButtonTooltip", [](SlotTypeFiltersComponentValue*) { return "Remove selected type filter"; }, nullptr)
        .Field("removeButtonEnabled", [](SlotTypeFiltersComponentValue* v) { return v->currentFilter.empty() == false; }, nullptr)
        .Method("removeTypeFilter", &SlotTypeFiltersComponentValue::RemoveTypeFilter)

        .Field("addFilterPopupText", &SlotTypeFiltersComponentValue::GetPopupText, &SlotTypeFiltersComponentValue::SetPopupText)
        .Field("filterEditPlaceholder", [](SlotTypeFiltersComponentValue*) { return "Type filter name"; }, nullptr)
        .End();
    }
};

class SlotJointComponentValue : public BaseSlotComponentValue
{
public:
    DAVA::Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const override
    {
        return false;
    }

    DAVA::TArc::ControlProxy* CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor) override
    {
        using namespace DAVA::TArc;

        ComboBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ComboBox::Fields::Enumerator] = "itemsList";
        params.fields[ComboBox::Fields::Value] = "currentJoint";
        params.fields[ComboBox::Fields::IsReadOnly] = BaseComponentValue::readOnlyFieldName;
        ComboBox* combo = new ComboBox(params, wrappersProcessor, model, parent);
        return combo;
    }

private:
    static const DAVA::String DetachItemName;
    const DAVA::Set<DAVA::String>& GetJoints() const
    {
        joints.clear();
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst)
                             {
                                 if (isFirst == true)
                                 {
                                     DAVA::SkeletonComponent* skeleton = GetSkeletonComponent(component->GetEntity());
                                     if (skeleton != nullptr)
                                     {
                                         for (DAVA::uint16 i = 0; i < skeleton->GetJointsCount(); ++i)
                                         {
                                             joints.insert(skeleton->GetJointName(i).c_str());
                                         }
                                     }
                                 }
                                 else
                                 {
                                     DAVA::Set<DAVA::String> jointIntersection;
                                     DAVA::SkeletonComponent* skeleton = GetSkeletonComponent(component->GetEntity());
                                     if (skeleton != nullptr)
                                     {
                                         for (DAVA::uint16 i = 0; i < skeleton->GetJointsCount(); ++i)
                                         {
                                             jointIntersection.insert(skeleton->GetJointName(i).c_str());
                                         }
                                     }

                                     std::swap(joints, jointIntersection);
                                 }
                                 return true;
                             });

        joints.emplace(DetachItemName);
        return joints;
    }

    DAVA::Any GetCurrentJoint() const
    {
        DAVA::FastName result(DAVA::TArc::MultipleValuesString);

        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst)
                             {
                                 if (isFirst == true)
                                 {
                                     result = component->GetJointName();
                                 }
                                 else
                                 {
                                     DAVA::FastName jointName = component->GetJointName();
                                     if (jointName != result)
                                     {
                                         result = DAVA::FastName(DAVA::TArc::MultipleValuesString);
                                         return false;
                                     }
                                 }

                                 return true;
                             });

        if (result.IsValid() == false)
        {
            return DAVA::Any(DetachItemName);
        }

        return DAVA::Any(DAVA::String(result.c_str()));
    }

    void SetCurrentJoint(const DAVA::Any& jointName)
    {
        if (jointName.IsEmpty())
        {
            return;
        }

        DAVA::String v = jointName.Cast<DAVA::String>();
        DAVA::Any newValue = DAVA::FastName(v);
        if (v == DetachItemName)
        {
            newValue = DAVA::FastName();
        }

        std::shared_ptr<DAVA::TArc::ModifyExtension> ext = GetModifyInterface();
        DAVA::TArc::ModifyExtension::MultiCommandInterface cmdInterface = ext->GetMultiCommandInterface("Attach to joint", static_cast<DAVA::uint32>(nodes.size()));
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst)
                             {
                                 DAVA::Reflection slotReflection = DAVA::Reflection::Create(DAVA::ReflectedObject(component));
                                 DAVA::Reflection jointNameField = slotReflection.GetField(DAVA::SlotComponent::AttchementToJointFieldName);
                                 DVASSERT(jointNameField.IsValid() == true);
                                 DAVA::Reflection::Field f;
                                 f.key = DAVA::SlotComponent::AttchementToJointFieldName;
                                 f.ref = jointNameField;
                                 cmdInterface.ProduceCommand(f, newValue);
                                 ;
                                 return true;
                             });
    }

    bool IsReadOnly() const override
    {
        if (BaseComponentValue::IsReadOnly() == true)
        {
            return true;
        }

        bool result = true;
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst)
                             {
                                 DAVA::SkeletonComponent* skeleton = GetSkeletonComponent(component->GetEntity());
                                 if (isFirst == true)
                                 {
                                     result = skeleton == nullptr;
                                 }
                                 else
                                 {
                                     result &= skeleton == nullptr;
                                 }
                                 return result;
                             });
        return result;
    }

    mutable DAVA::Set<DAVA::String> joints;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotJointComponentValue, BaseSlotComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotJointComponentValue>::Begin()
        .Field("itemsList", &SlotJointComponentValue::GetJoints, nullptr)
        .Field("currentJoint", &SlotJointComponentValue::GetCurrentJoint, &SlotJointComponentValue::SetCurrentJoint)
        .End();
    }
};

const DAVA::String SlotJointComponentValue::DetachItemName = DAVA::String("< Detached >");

class SlotPreviewComponentValue : public BaseSlotComponentValue
{
public:
    DAVA::Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const override
    {
        return false;
    }

    DAVA::TArc::ControlProxy* CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor) override
    {
        using namespace DAVA::TArc;

        ComboBox::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ComboBox::Fields::Enumerator] = "itemsList";
        params.fields[ComboBox::Fields::Value] = "currentPreviewItem";
        params.fields[ComboBox::Fields::IsReadOnly] = "previewItemReadOnly";
        ComboBox* combo = new ComboBox(params, wrappersProcessor, model, parent);
        return combo;
    }

private:
    void UpdateValues() const
    {
        DAVA::Any currentConfig;
        DAVA::Set<DAVA::FastName> currentFilters;
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst) {
            if (isFirst == true)
            {
                currentConfig = DAVA::Any(component->GetConfigFilePath());
                for (DAVA::uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
                {
                    currentFilters.insert(component->GetTypeFilter(i));
                }
            }
            else
            {
                if (currentConfig != component->GetConfigFilePath())
                {
                    currentConfig = DAVA::TArc::MultipleValuesString;
                }

                DAVA::Set<DAVA::FastName> filtersIntersection;
                for (DAVA::uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
                {
                    DAVA::FastName typeFilter = component->GetTypeFilter(i);
                    if (currentFilters.count(typeFilter) > 0)
                    {
                        filtersIntersection.insert(typeFilter);
                    }
                }

                std::swap(currentFilters, filtersIntersection);
            }
            return true;
        });

        if (currentConfig != configPath || currentFilters != filters)
        {
            configPath = currentConfig;
            filters = currentFilters;
            RebuildItemsList();
        }
        else
        {
            DAVA::RefPtr<SceneEditor2> scene = GetAccessor()->GetActiveContext()->GetData<SceneData>()->GetScene();
            if (configPath.CanGet<DAVA::FilePath>() && scene->slotSystem->IsConfigParsed(configPath.Get<DAVA::FilePath>()) == false)
            {
                RebuildItemsList();
            }
        }
    }

    DAVA::String GetCurrentItem() const
    {
        DAVA::FastName item = GetLoadedItemInfo();
        DVASSERT(item.IsValid());
        DAVA::String strItem(item.c_str());
        if (itemsList.find(strItem) == itemsList.end())
        {
            RebuildItemsList();
        }
        return strItem;
    }

    void SetCurrentItem(const DAVA::String& item)
    {
        using namespace DAVA::TArc;

        DAVA::FastName itemName(item);
        DAVA::RefPtr<SceneEditor2> scene = GetAccessor()->GetActiveContext()->GetData<SceneData>()->GetScene();

        std::shared_ptr<ModifyExtension> extension = GetModifyInterface();
        ModifyExtension::MultiCommandInterface cmdInterface = extension->GetMultiCommandInterface("Load preview item to slot", static_cast<DAVA::uint32>(nodes.size()));
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool) {
            cmdInterface.Exec(std::make_unique<AttachEntityToSlot>(scene.Get(), component, itemName));
            return true;
        });

        RebuildItemsList();
    }

    bool IsPreviewReadOnly() const
    {
        UpdateValues();
        return configPath.CanGet<DAVA::FilePath>() == false;
    }

    DAVA::FastName GetLoadedItemInfo() const
    {
        DAVA::FastName item;
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst) {
            DAVA::FastName loadedItem = component->GetLoadedItemName();
            if (isFirst == true)
            {
                item = loadedItem;
            }
            else if (item != loadedItem)
            {
                item = DAVA::FastName(DAVA::TArc::MultipleValuesString);
                return false;
            }

            return true;
        });

        if (item.IsValid() == false)
        {
            return EditorSlotSystem::emptyItemName;
        }

        return item;
    }

    void RebuildItemsList() const
    {
        using namespace DAVA;

        itemsList.clear();
        itemsList.emplace(EditorSlotSystem::emptyItemName.c_str(), EditorSlotSystem::emptyItemName);
        DAVA::FastName item = GetLoadedItemInfo();
        if (configPath.CanGet<DAVA::FilePath>() == true)
        {
            DAVA::FilePath path = configPath.Get<DAVA::FilePath>();
            DAVA::RefPtr<SceneEditor2> scene = GetAccessor()->GetActiveContext()->GetData<SceneData>()->GetScene();

            Vector<DAVA::SlotSystem::ItemsCache::Item> items = scene->slotSystem->GetItems(path);
            for (const DAVA::SlotSystem::ItemsCache::Item& item : items)
            {
                if (filters.empty() == true || filters.count(item.type) > 0)
                {
                    itemsList.emplace(item.itemName.c_str(), item.itemName);
                }
            }
        }

        if (itemsList.find(item.c_str()) == itemsList.end())
        {
            itemsList.emplace(item.c_str(), item);
        }
    }

    const DAVA::Map<DAVA::String, DAVA::FastName>& GetItemsList() const
    {
        UpdateValues();
        return itemsList;
    }

    mutable DAVA::Any configPath;
    mutable DAVA::Set<DAVA::FastName> filters;
    mutable DAVA::Map<DAVA::String, DAVA::FastName> itemsList;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotPreviewComponentValue, BaseSlotComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotPreviewComponentValue>::Begin()
        .Field("currentPreviewItem", &SlotPreviewComponentValue::GetCurrentItem, &SlotPreviewComponentValue::SetCurrentItem)
        .Field("previewItemReadOnly", &SlotPreviewComponentValue::IsPreviewReadOnly, nullptr)
        .Field("itemsList", &SlotPreviewComponentValue::GetItemsList, nullptr)
        .End();
    }
};

class SlotTemplateComponentValue : public DAVA::TArc::BaseComponentValue
{
public:
    DAVA::Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const DAVA::Any& newValue, const DAVA::Any& currentValue) const override
    {
        if (currentValue.IsEmpty())
        {
            return true;
        }

        return newValue.IsEmpty() == true || newValue != currentValue;
    }

    DAVA::TArc::ControlProxy* CreateEditorWidget(QWidget* parent, const DAVA::Reflection& model, DAVA::TArc::DataWrappersProcessor* wrappersProcessor) override
    {
        DAVA::TArc::ContextAccessor* accessor = GetAccessor();
        SlotTemplatesData* data = accessor->GetGlobalContext()->GetData<SlotTemplatesData>();
        DAVA::Vector<SlotTemplatesData::Template> templates = data->GetTemplates();
        for (const SlotTemplatesData::Template& t : templates)
        {
            enumerator.emplace(t.name.c_str());
        }

        if (enumerator.empty() == true)
        {
            enumerator.emplace("");
            isEnumeratorEmpty = true;
        }

        DAVA::TArc::ComboBox::Params params(accessor, GetUI(), GetWindowKey());
        params.fields[DAVA::TArc::ComboBox::Fields::Enumerator] = "enumerator";
        params.fields[DAVA::TArc::ComboBox::Fields::Value] = "value";
        params.fields[DAVA::TArc::ComboBox::Fields::IsReadOnly] = "isEmpty";
        return new DAVA::TArc::ComboBox(params, wrappersProcessor, model, parent);
    }

private:
    DAVA::Any GetTemplateName() const
    {
        DAVA::Any fastNameAny = GetValue();
        if (fastNameAny.IsEmpty())
        {
            return fastNameAny;
        }

        return fastNameAny.Cast<DAVA::String>();
    }

    void SetTemplateName(const DAVA::Any& templateName)
    {
        SetValue(templateName);
    }

    DAVA::Set<DAVA::String> enumerator;
    bool isEnumeratorEmpty = false;
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotTemplateComponentValue, DAVA::TArc::BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotTemplateComponentValue>::Begin()
        .Field("enumerator", &SlotTemplateComponentValue::enumerator)
        .Field("value", &SlotTemplateComponentValue::GetTemplateName, &SlotTemplateComponentValue::SetTemplateName)
        .Field("isEmpty", &SlotTemplateComponentValue::isEnumeratorEmpty)
        .End();
    }
};

class SlotNameComponentValue : public DAVA::TArc::TextComponentValue
{
public:
    bool IsReadOnly() const override
    {
        return DAVA::TArc::TextComponentValue::IsReadOnly() || GetAccessor()->GetGlobalContext()->GetData<SlotSystemSettings>()->autoGenerateSlotNames;
    }
};

void SlotComponentChildCreator::ExposeChildren(const std::shared_ptr<DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const
{
    if (parent->propertyType == SlotPreviewProperty ||
        parent->propertyType == SlotTypeFilters ||
        parent->propertyType == SlotJointAttachment)
    {
        return;
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
    const DAVA::ReflectedType* fieldType = DAVA::TArc::GetValueReflectedType(parent->field.ref);
    const DAVA::ReflectedType* slotComponentType = DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>();
    if (fieldType == slotComponentType)
    {
        {
            auto iter = std::find_if(children.rbegin(), children.rend(), [](const std::shared_ptr<DAVA::TArc::PropertyNode>& node)
                                     {
                                         return node->field.key.Cast<DAVA::FastName>() == DAVA::SlotComponent::AttchementToJointFieldName;
                                     });

            DVASSERT(iter != children.rend());
            std::shared_ptr<DAVA::TArc::PropertyNode> jointAttachmentNode = *iter;
            jointAttachmentNode->propertyType = SlotJointAttachment;
            const DAVA::M::DisplayName* displayNameMeta = jointAttachmentNode->field.ref.GetMeta<DAVA::M::DisplayName>();
            if (displayNameMeta != nullptr)
            {
                jointAttachmentNode->field.key = displayNameMeta->displayName;
            }
            jointAttachmentNode->field.ref = parent->field.ref;
            jointAttachmentNode->cachedValue = parent->cachedValue;
        }

        {
            auto iter = std::find_if(children.rbegin(), children.rend(), [](const std::shared_ptr<DAVA::TArc::PropertyNode>& node)
                                     {
                                         return node->field.key.Cast<DAVA::FastName>() == DAVA::SlotComponent::TemplateFieldName;
                                     });

            DVASSERT(iter != children.rend());
            std::shared_ptr<DAVA::TArc::PropertyNode> templateNameNode = *iter;
            templateNameNode->propertyType = SlotTemplateName;
        }

        {
            if (IsDeveloperMode() == true)
            {
                DAVA::Reflection::Field f;
                f.key = DAVA::FastName("Type Filters");
                f.ref = parent->field.ref;
                std::shared_ptr<DAVA::TArc::PropertyNode> previewNode = allocator->CreatePropertyNode(parent, std::move(f), static_cast<DAVA::int32>(children.size()), SlotTypeFilters);
                children.push_back(previewNode);
            }
        }

        {
            DAVA::Reflection::Field f;
            f.key = DAVA::FastName("Loaded item");
            f.ref = parent->field.ref;
            std::shared_ptr<DAVA::TArc::PropertyNode> previewNode = allocator->CreatePropertyNode(parent, std::move(f), static_cast<DAVA::int32>(children.size()), SlotPreviewProperty);
            children.push_back(previewNode);
        }
    }
}

std::unique_ptr<DAVA::TArc::BaseComponentValue> SlotComponentEditorCreator::GetEditor(const std::shared_ptr<const DAVA::TArc::PropertyNode>& node) const
{
    if (node->propertyType == SlotTypeFilters)
    {
        return std::make_unique<SlotTypeFiltersComponentValue>();
    }

    if (node->propertyType == SlotPreviewProperty)
    {
        return std::make_unique<SlotPreviewComponentValue>();
    }

    if (node->propertyType == SlotJointAttachment)
    {
        return std::make_unique<SlotJointComponentValue>();
    }

    if (node->propertyType == SlotTemplateName)
    {
        return std::make_unique<SlotTemplateComponentValue>();
    }

    if (node->field.key == DAVA::SlotComponent::SlotNameFieldName)
    {
        DAVA::ReflectedObject obj = node->field.ref.GetDirectObject();
        if (obj.GetReflectedType() == DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>())
        {
            return std::make_unique<SlotNameComponentValue>();
        }
    }

    return EditorComponentExtension::GetEditor(node);
}

class CloneSlotComponent : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::TArc::PropertyNode>& node) const override
    {
        return true;
    }

    Info GetInfo() const override
    {
        Info info;
        info.description = "Slot component cloned";
        info.tooltip = "Clone current component";
        info.icon = DAVA::TArc::SharedIcon(":/QtIcons/clone_inplace.png");

        return info;
    }

    std::unique_ptr<DAVA::Command> CreateCommand(const std::shared_ptr<DAVA::TArc::PropertyNode>& node, const Params& params) const override
    {
        DAVA::SlotComponent* component = node->field.ref.GetValueObject().GetPtr<DAVA::SlotComponent>();
        DAVA::Component* clonedComponent = component->Clone(nullptr);
        return std::make_unique<AddComponentCommand>(component->GetEntity(), clonedComponent);
    }
};

class GenerateUniqueName : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::TArc::PropertyNode>& node) const override
    {
        return true;
    }

    Info GetInfo() const override
    {
        Info info;
        info.description = "Unique slot name generated";
        info.tooltip = "Generate unique slot name";
        info.icon = DAVA::TArc::SharedIcon(":/QtIcons/rebuild_name.png");

        return info;
    }

    std::unique_ptr<DAVA::Command> CreateCommand(const std::shared_ptr<DAVA::TArc::PropertyNode>& node, const Params& params) const override
    {
        DAVA::ReflectedObject slotObject = node->field.ref.GetDirectObject();
        DAVA::SlotComponent* component = slotObject.GetPtr<DAVA::SlotComponent>();

        DAVA::FastName name = EditorSlotSystem::GenerateUniqueSlotName(component);

        return std::make_unique<SetFieldValueCommand>(node->field, name);
    }
};

class InvalidateSlotConfigCache : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::TArc::PropertyNode>& node) const override
    {
        return true;
    }

    Info GetInfo() const override
    {
        Info info;
        info.description = "Slot config reloading";
        info.tooltip = "Reload config from disk";
        info.icon = DAVA::TArc::SharedIcon(":/QtIcons/reloadtextures.png");

        return info;
    }

    std::unique_ptr<DAVA::Command> CreateCommand(const std::shared_ptr<DAVA::TArc::PropertyNode>& node, const Params& params) const override
    {
        DAVA::TArc::DataContext* ctx = params.accessor->GetActiveContext();
        DVASSERT(ctx != nullptr);

        DAVA::Any value = node->field.ref.GetValue();
        if (value.CanCast<DAVA::FilePath>())
        {
            DAVA::FilePath path = value.Cast<DAVA::FilePath>();
            SceneData* data = ctx->GetData<SceneData>();
            DVASSERT(data != nullptr);
            data->GetScene()->slotSystem->InvalidateConfig(path);
        }
        return nullptr;
    }

private:
    DAVA::UnorderedMap<DAVA::RenderObject*, DAVA::Entity*> cache;
};

std::shared_ptr<DAVA::M::CommandProducer> CreateCloneSlotProducer()
{
    return std::make_shared<CloneSlotComponent>();
}

DAVA::M::CommandProducerHolder CreateSlotNameCommandProvider()
{
    DAVA::M::CommandProducerHolder holder;
    holder.AddCommandProducer(std::make_shared<GenerateUniqueName>());

    return holder;
}

DAVA::M::CommandProducerHolder CreateSlotConfigCommandProvider()
{
    DAVA::M::CommandProducerHolder holder;
    holder.AddCommandProducer(std::make_shared<InvalidateSlotConfigCache>());

    return holder;
}
}
