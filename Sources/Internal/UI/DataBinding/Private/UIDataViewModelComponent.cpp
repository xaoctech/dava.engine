#include "UI/DataBinding/UIDataViewModelComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIDataViewModelComponent)
{
    ReflectionRegistrator<UIDataViewModelComponent>::Begin()[M::DisplayName("Data View Model"), M::Group("Data")]
    .ConstructorByPointer()
    .DestructorByPointer([](UIDataViewModelComponent* o) { o->Release(); })
    .Field("viewModel", &UIDataViewModelComponent::GetViewModelFile, &UIDataViewModelComponent::SetViewModelFile)[M::DisplayName("View Model")]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIDataViewModelComponent);

UIDataViewModelComponent::UIDataViewModelComponent(const UIDataViewModelComponent& c)
    : viewModelFile(c.viewModelFile)
{
    isDirty = true;
}

UIDataViewModelComponent* UIDataViewModelComponent::Clone() const
{
    return new UIDataViewModelComponent(*this);
}

const FilePath& UIDataViewModelComponent::GetViewModelFile() const
{
    return viewModelFile;
}

void UIDataViewModelComponent::SetViewModelFile(const FilePath& path)
{
    viewModelFile = path;
    isDirty = true;
}

bool UIDataViewModelComponent::IsDirty() const
{
    return isDirty;
}

void UIDataViewModelComponent::SetDirty(bool dirty_)
{
    isDirty = dirty_;
}
}
