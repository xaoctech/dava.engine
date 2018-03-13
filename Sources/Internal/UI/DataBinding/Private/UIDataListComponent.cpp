#include "UI/DataBinding/UIDataListComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIDataListComponent)
{
    ReflectionRegistrator<UIDataListComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIDataListComponent* o) { o->Release(); })
    .Field("cellPackage", &UIDataListComponent::GetCellPackage, &UIDataListComponent::SetCellPackage)
    .Field("cellControl", &UIDataListComponent::GetCellControlName, &UIDataListComponent::SetCellControlName)
    .Field("dataContainer", &UIDataListComponent::GetDataContainer, &UIDataListComponent::SetDataContainer)
    .Field("selectionSupported", &UIDataListComponent::IsSelectionSupported, &UIDataListComponent::SetSelectionSupported)
    .Field("selectedIndex", &UIDataListComponent::GetSelectedIndex, &UIDataListComponent::SetSelectedIndex)[M::Bindable()]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIDataListComponent);

UIDataListComponent::UIDataListComponent(const UIDataListComponent& c)
    : cellPackage(c.cellPackage)
    , cellControlName(c.cellControlName)
    , dataContainer(c.dataContainer)
    , selectionSupported(c.selectionSupported)
    , selectedIndex(c.selectedIndex)
{
    isDirty = true;
}

UIDataListComponent* UIDataListComponent::Clone() const
{
    return new UIDataListComponent(*this);
}

const FilePath& UIDataListComponent::GetCellPackage() const
{
    return cellPackage;
}

void UIDataListComponent::SetCellPackage(const FilePath& package)
{
    cellPackage = package;
    isDirty = true;
}

const String& UIDataListComponent::GetCellControlName() const
{
    return cellControlName;
}

void UIDataListComponent::SetCellControlName(const String& control_)
{
    cellControlName = control_;
    isDirty = true;
}

const String& UIDataListComponent::GetDataContainer() const
{
    return dataContainer;
}

void UIDataListComponent::SetDataContainer(const String& dataContainer_)
{
    dataContainer = dataContainer_;

    if (dataContainer.length() > 0 && dataContainer.at(0) == '=')
    {
        dataContainer = dataContainer.substr(1);
    }

    isDirty = true;
}

bool UIDataListComponent::IsDirty() const
{
    return isDirty;
}

void UIDataListComponent::SetDirty(bool dirty_)
{
    isDirty = dirty_;
}

bool UIDataListComponent::IsSelectionSupported() const
{
    return selectionSupported;
}

void UIDataListComponent::SetSelectionSupported(bool supported)
{
    selectionSupported = supported;
    isDirty = true;
}

int32 UIDataListComponent::GetSelectedIndex() const
{
    return selectedIndex;
}

void UIDataListComponent::SetSelectedIndex(int32 index)
{
    selectedIndex = index;
}
}
