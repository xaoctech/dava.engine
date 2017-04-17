#include "Classes/SlotSupportModule/Private/SlotComponentExtensions.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"

#include <TArc/Utils/ReflectionHelpers.h>

#include <FileSystem/FilePath.h>

namespace PropertyPanel
{
class SlotPreviewComponentValue : public DAVA::TArc::BaseComponentValue
{
public:
    SlotPreviewComponentValue() = default;

    DAVA::Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override
    {
        return false;
    }

    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
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
    }

    const Vector<String> GetItemsList()
    {
        std::shared_ptr<DAVA::TArc::PropertyNode> node = nodes.front();
        SlotComponent* firstComponent = node->field.ref.GetValueObject().GetPtr<DAVA::SlotComponent>();
        if (configPath != component->GetConfigPath())
        {
            itemsList.clear();
            itemsList.push_back(String("Choose item for loading"));
            configPath = firstComponent->GetConfigPath();
            for (size_t i = 1; i < nodes.size(); ++i)
            {
                SlotComponent* nextComponent = nodes[i]->field.ref.GetValueObject().GetPtr<DAVA::SlotComponent>();
                if (configPath != nextComponent->GetConfigPath())
                {
                    configPath = DAVA::FilePath();
                    break;
                }
            }

            if (configPath.IsEmpty() == false)
            {
                DAVA::Scene* scene = firstComponent->GetEntity()->GetScene();
            }
        }

        return itemsList;
    }

    FilePath configPath;
    Vector<String> itemsList;

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
    if (parent->propertyType == SlotPreviewProperty)
    {
        return;
    }

    return EditorComponentExtension::GetEditor(node);
}
}
