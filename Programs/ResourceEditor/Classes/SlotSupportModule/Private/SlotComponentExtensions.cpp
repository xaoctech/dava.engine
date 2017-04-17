#include "Classes/SlotSupportModule/Private/SlotComponentExtensions.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"

#include <TArc/Utils/ReflectionHelpers.h>
#include <TArc/Controls/ComboBox.h>

#include <FileSystem/FilePath.h>
#include <Base/BaseTypes.h>
#include "Commands2/SlotCommands.h"
#include "Scene/SceneEditor2.h"

namespace PropertyPanel
{
class SlotPreviewComponentValue : public DAVA::TArc::BaseComponentValue
{
public:
    SlotPreviewComponentValue()
    {
        itemsList.push_back(String("Choose item for loading"));
    }

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
        DAVA::TArc::ControlDescriptorBuilder<DAVA::TArc::ComboBox::Fields> descr;
        descr[DAVA::TArc::ComboBox::Fields::Enumerator] = "itemsList";
        descr[DAVA::TArc::ComboBox::Fields::Value] = "value";
        descr[DAVA::TArc::ComboBox::Fields::IsReadOnly] = "isReadOnly";
        return new DAVA::TArc::ComboBox(descr, wrappersProcessor, model, parent);
    }

private:
    bool IsReadOnly() const
    {
        return configPath.IsEmpty();
    }

    size_t GetItemIndex() const
    {
        return 0;
    }

    void SetItemIndex(size_t index)
    {
        using namespace DAVA::TArc;

        if (IsReadOnly() == true)
        {
            return;
        }

        if (index > 0)
        {
            DVASSERT(index < itemsList.size());
            std::shared_ptr<ModifyExtension> extension = GetModifyInterface();
            ModifyExtension::MultiCommandInterface cmdInterface = extension->GetMultiCommandInterface("Load preview item to slot", static_cast<DAVA::uint32>(nodes.size()));
            DAVA::FastName itemToLoad = DAVA::FastName(itemsList[index]);

            for (const std::shared_ptr<PropertyNode>& node : nodes)
            {
                DAVA::Component* component = node->cachedValue.Cast<DAVA::Component*>();
                DVASSERT(component->GetType() == DAVA::Component::SLOT_COMPONENT);

                DAVA::SlotComponent* slotComponent = static_cast<SlotComponent*>(component);

                SceneEditor2* sceneEditor = DAVA::DynamicTypeCheck<SceneEditor2*>(slotComponent->GetEntity()->GetScene());
                cmdInterface.Exec(std::make_unique<AttachEntityToSlot>(sceneEditor, slotComponent, itemToLoad));
            }
        }
    }

    const DAVA::Vector<DAVA::String>& GetItemsList() const
    {
        std::shared_ptr<DAVA::TArc::PropertyNode> node = nodes.front();
        DAVA::Component* component = node->cachedValue.Cast<DAVA::Component*>();
        DVASSERT(component->GetType() == DAVA::Component::SLOT_COMPONENT);

        DAVA::SlotComponent* firstComponent = static_cast<SlotComponent*>(component);
        if (configPath != firstComponent->GetConfigFilePath())
        {
            itemsList.resize(1);
            configPath = firstComponent->GetConfigFilePath();
            for (size_t i = 1; i < nodes.size(); ++i)
            {
                component = nodes[i]->cachedValue.Cast<DAVA::Component*>();
                DVASSERT(component->GetType() == DAVA::Component::SLOT_COMPONENT);

                DAVA::SlotComponent* nextComponent = static_cast<SlotComponent*>(component);
                if (configPath != nextComponent->GetConfigFilePath())
                {
                    configPath = DAVA::FilePath();
                    break;
                }
            }

            if (configPath.IsEmpty() == false)
            {
                DAVA::Scene* scene = firstComponent->GetEntity()->GetScene();
                DAVA::Vector<DAVA::SlotSystem::ItemsCache::Item> items = scene->slotSystem->GetItems(configPath);
                itemsList.reserve(items.size() + 1);
                for (const DAVA::SlotSystem::ItemsCache::Item& item : items)
                {
                    itemsList.push_back(item.itemName.c_str());
                }
            }
        }

        return itemsList;
    }

    mutable DAVA::FilePath configPath;
    mutable DAVA::Vector<DAVA::String> itemsList;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotPreviewComponentValue, DAVA::TArc::BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<SlotPreviewComponentValue>::Begin()
        .Field("isReadOnly", &SlotPreviewComponentValue::IsReadOnly, nullptr)
        .Field("value", &SlotPreviewComponentValue::GetItemIndex, &SlotPreviewComponentValue::SetItemIndex)
        .Field("itemsList", &SlotPreviewComponentValue::GetItemsList, nullptr)
        .End();
    }
};

void SlotComponentChildCreator::ExposeChildren(const std::shared_ptr<DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const
{
    if (parent->propertyType == SlotPreviewProperty)
    {
        return;
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
    const DAVA::ReflectedType* fieldType = DAVA::TArc::GetValueReflectedType(parent->field.ref);
    const DAVA::ReflectedType* slotComponentType = DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>();
    if (fieldType == slotComponentType)
    {
        DAVA::Reflection::Field f;
        f.key = DAVA::FastName("Load preview");
        f.ref = parent->field.ref;
        std::shared_ptr<DAVA::TArc::PropertyNode> previewNode = allocator->CreatePropertyNode(parent, std::move(f), SlotPreviewProperty);
        children.push_back(previewNode);
    }
}

std::unique_ptr<DAVA::TArc::BaseComponentValue> SlotComponentEditorCreator::GetEditor(const std::shared_ptr<const DAVA::TArc::PropertyNode>& node) const
{
    if (node->propertyType == SlotPreviewProperty)
    {
        return std::make_unique<SlotPreviewComponentValue>();
    }

    return EditorComponentExtension::GetEditor(node);
}
}
