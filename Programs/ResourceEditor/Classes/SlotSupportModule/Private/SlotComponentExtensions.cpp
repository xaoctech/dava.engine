#include "Classes/SlotSupportModule/Private/SlotComponentExtensions.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"
#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/Controls/CommonStrings.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/FilePathEdit.h>
#include <TArc/Controls/Widget.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Controls/ListView.h>
#include <TArc/Controls/PopupLineEdit.h>
#include <TArc/Utils/ReflectionHelpers.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <FileSystem/FilePath.h>
#include <Base/BaseTypes.h>

namespace PropertyPanel
{
class BaseSlotComponentValue : public DAVA::TArc::BaseComponentValue
{
protected:
    void ForEachSlotComponent(const DAVA::Function<bool(SlotComponent*, bool)>& fn) const
    {
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            std::shared_ptr<DAVA::TArc::PropertyNode> node = nodes[i];
            SlotComponent* component = node->cachedValue.Cast<SlotComponent*>(nullptr);
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

        ControlDescriptorBuilder<ListView::Fields> descr;
        descr[ListView::Fields::ValueList] = "filtersList";
        descr[ListView::Fields::CurrentValue] = "currentFilter";
        ListView* filtersControl = new ListView(descr, wrappersProcessor, model, parent);
        filtersControl->ToWidgetCast()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        w->AddControl(filtersControl);

        Widget* buttonsBar = new Widget(w->ToWidgetCast());
        buttonsBar->SetLayout(new QVBoxLayout());
        w->AddControl(buttonsBar);

        {
            ControlDescriptorBuilder<ReflectedButton::Fields> descr;
            descr[ReflectedButton::Fields::AutoRaise] = "autoRise";
            descr[ReflectedButton::Fields::Icon] = "addButtonIcon";
            descr[ReflectedButton::Fields::ToolTip] = "addButtonTooltip";
            descr[ReflectedButton::Fields::Clicked] = "addTypeFilter";
            ReflectedButton* addButton = new ReflectedButton(descr, wrappersProcessor, model, w->ToWidgetCast());
            buttonsBar->AddControl(addButton);
        }

        {
            ControlDescriptorBuilder<ReflectedButton::Fields> descr;
            descr[ReflectedButton::Fields::AutoRaise] = "autoRise";
            descr[ReflectedButton::Fields::Icon] = "removeButtonIcon";
            descr[ReflectedButton::Fields::ToolTip] = "removeButtonTooltip";
            descr[ReflectedButton::Fields::Enabled] = "removeButtonEnabled";
            descr[ReflectedButton::Fields::Clicked] = "removeTypeFilter";
            ReflectedButton* addButton = new ReflectedButton(descr, wrappersProcessor, model, w->ToWidgetCast());
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
            for (uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
            {
                FastName filter = component->GetTypeFilter(i);
                if (intersection.empty() == true || intersection.count(filter) > 0)
                {
                    currentIntersection.insert(component->GetTypeFilter(i));
                }
            }

            std::swap(intersection, currentIntersection);
            return true;
        });

        filters.clear();
        std::transform(intersection.begin(), intersection.end(), std::inserter(filters, filters.end()), DAVA::Bind(&FastName::c_str, DAVA::_1));
        return filters;
    }

    void AddTypeFilter()
    {
        using namespace DAVA::TArc;

        ControlDescriptorBuilder<LineEdit::Fields> descr;
        descr[LineEdit::Fields::Text] = "addFilterPopupText";
        descr[LineEdit::Fields::PlaceHolder] = "filterEditPlaceholder";

        DAVA::Reflection popupModel = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
        DAVA::TArc::PopupLineEdit* popupLineEdit = new DAVA::TArc::PopupLineEdit(descr, GetDataProcessor(), popupModel, realWidget);
        popupLineEdit->Show(realWidget->parentWidget()->mapToGlobal(realWidget->geometry().topLeft()));
    }

    void RemoveTypeFilter()
    {
        DAVA::FastName filterToRemove(currentFilter);
        DAVA::String descr = DAVA::Format("Remove type filter: %s", currentFilter.c_str());
        DAVA::TArc::ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(descr, static_cast<uint32>(nodes.size()));
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst)
                             {
                                 cmdInterface.Exec(std::make_unique<SlotTypeFilterEdit>(component, filterToRemove, false));
                                 return true;
                             });
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
        DAVA::TArc::ModifyExtension::MultiCommandInterface cmdInterface = GetModifyInterface()->GetMultiCommandInterface(descr, static_cast<uint32>(nodes.size()));
        ForEachSlotComponent([&](DAVA::SlotComponent* component, bool isFirst)
                             {
                                 cmdInterface.Exec(std::make_unique<SlotTypeFilterEdit>(component, filterToAdd, true));
                                 return true;
                             });
    }

    mutable DAVA::Set<DAVA::String> filters;
    DAVA::String currentFilter = DAVA::String("");

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotTypeFiltersComponentValue, BaseSlotComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotTypeFiltersComponentValue>::Begin()
        .Field("filtersList", &SlotTypeFiltersComponentValue::GetTypeFilters, nullptr)
        .Field("currentFilter", &SlotTypeFiltersComponentValue::currentFilter)
        .Field("autoRise", [](SlotTypeFiltersComponentValue*) { return false; }, nullptr)
        .Field("addButtonIcon", [](SlotTypeFiltersComponentValue*) { return SharedIcon(":/QtIcons/cplus.png"); }, nullptr)
        .Field("addButtonTooltip", [](SlotTypeFiltersComponentValue*) { return "Add type filter"; }, nullptr)
        .Method("addTypeFilter", &SlotTypeFiltersComponentValue::AddTypeFilter)
        .Field("removeButtonIcon", [](SlotTypeFiltersComponentValue*) { return SharedIcon(":/QtIcons/cminus.png"); }, nullptr)
        .Field("removeButtonTooltip", [](SlotTypeFiltersComponentValue*) { return "Remove selected type filter"; }, nullptr)
        .Field("removeButtonEnabled", [](SlotTypeFiltersComponentValue* v) { return v->currentFilter.empty() == false; }, nullptr)
        .Method("removeTypeFilter", &SlotTypeFiltersComponentValue::RemoveTypeFilter)

        .Field("addFilterPopupText", &SlotTypeFiltersComponentValue::GetPopupText, &SlotTypeFiltersComponentValue::SetPopupText)
        .Field("filterEditPlaceholder", [](SlotTypeFiltersComponentValue*) { return "Type filter name"; }, nullptr)
        .End();
    }
};

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

        ControlDescriptorBuilder<ComboBox::Fields> descr;
        descr[ComboBox::Fields::Enumerator] = "itemsList";
        descr[ComboBox::Fields::Value] = "currentPreviewItem";
        descr[ComboBox::Fields::IsReadOnly] = "previewItemReadOnly";
        ComboBox* combo = new ComboBox(descr, wrappersProcessor, model, parent);
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
                for (uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
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
                for (uint32 i = 0; i < component->GetTypeFiltersCount(); ++i)
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
    }

    size_t GetCurrentItemIndex() const
    {
        FastName item = GetLoadedItemInfo();
        auto iter = std::find(itemsList.begin(), itemsList.end(), item);
        if (iter == itemsList.end())
        {
            UpdateValues();
            iter = std::find(itemsList.begin(), itemsList.end(), item);
        }
        DVASSERT(iter != itemsList.end());
        return std::distance(itemsList.begin(), iter);
    }

    void SetCurrentItemIndex(size_t index)
    {
        using namespace DAVA::TArc;

        FastName itemName = itemsList[index];
        DAVA::RefPtr<SceneEditor2> scene = GetAccessor()->GetActiveContext()->GetData<SceneData>()->GetScene();

        std::shared_ptr<ModifyExtension> extension = GetModifyInterface();
        ModifyExtension::MultiCommandInterface cmdInterface = extension->GetMultiCommandInterface("Load preview item to slot", static_cast<DAVA::uint32>(nodes.size()));
        ForEachSlotComponent([&](SlotComponent* component, bool) {
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

    FastName GetLoadedItemInfo() const
    {
        FastName item;
        ForEachSlotComponent([&](SlotComponent* component, bool isFirst) {
            FastName loadedItem = component->GetLoadedItemName();
            if (isFirst == true)
            {
                item = loadedItem;
            }
            else if (item != loadedItem)
            {
                item = FastName(DAVA::TArc::MultipleValuesString);
                return false;
            }

            return true;
        });

        return item;
    }

    void RebuildItemsList() const
    {
        using namespace DAVA;

        itemsList.clear();
        itemsList.push_back(EditorSlotSystem::emptyItemName);
        FastName item = GetLoadedItemInfo();
        if (configPath.CanGet<DAVA::FilePath>() == true)
        {
            FilePath path = configPath.Get<FilePath>();
            RefPtr<SceneEditor2> scene = GetAccessor()->GetActiveContext()->GetData<SceneData>()->GetScene();

            Vector<SlotSystem::ItemsCache::Item> items = scene->slotSystem->GetItems(path);
            for (const SlotSystem::ItemsCache::Item& item : items)
            {
                if (filters.empty() == true || filters.count(item.type) > 0)
                {
                    itemsList.push_back(item.itemName);
                }
            }
        }

        if (std::find(itemsList.begin(), itemsList.end(), item) == itemsList.end())
        {
            itemsList.push_back(item);
        }
    }

    const DAVA::Vector<DAVA::FastName>& GetItemsList() const
    {
        UpdateValues();
        return itemsList;
    }

    mutable DAVA::Any configPath;
    mutable DAVA::Set<DAVA::FastName> filters;
    mutable DAVA::Vector<DAVA::FastName> itemsList;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotPreviewComponentValue, BaseSlotComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotPreviewComponentValue>::Begin()
        .Field("currentPreviewItem", &SlotPreviewComponentValue::GetCurrentItemIndex, &SlotPreviewComponentValue::SetCurrentItemIndex)
        .Field("previewItemReadOnly", &SlotPreviewComponentValue::IsPreviewReadOnly, nullptr)
        .Field("itemsList", &SlotPreviewComponentValue::GetItemsList, nullptr)
        .End();
    }
};

void SlotComponentChildCreator::ExposeChildren(const std::shared_ptr<DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const
{
    if (parent->propertyType == SlotPreviewProperty ||
        parent->propertyType == SlotTypeFilters)
    {
        return;
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
    const DAVA::ReflectedType* fieldType = DAVA::TArc::GetValueReflectedType(parent->field.ref);
    const DAVA::ReflectedType* slotComponentType = DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>();
    if (fieldType == slotComponentType)
    {
        {
            DAVA::Reflection::Field f;
            f.key = DAVA::FastName("TypeFilters");
            f.ref = parent->field.ref;
            std::shared_ptr<DAVA::TArc::PropertyNode> previewNode = allocator->CreatePropertyNode(parent, std::move(f), static_cast<int32>(children.size()), SlotTypeFilters);
            children.push_back(previewNode);
        }

        {
            DAVA::Reflection::Field f;
            f.key = DAVA::FastName("Loaded item");
            f.ref = parent->field.ref;
            std::shared_ptr<DAVA::TArc::PropertyNode> previewNode = allocator->CreatePropertyNode(parent, std::move(f), static_cast<int32>(children.size()), SlotPreviewProperty);
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

    return EditorComponentExtension::GetEditor(node);
}
}
