#include "UI/DataBinding/UIDataSourceComponent.h"

#include "Engine/Engine.h"
#include "Entity/ComponentManager.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/StringUtils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIDataSourceComponent)
{
    ReflectionRegistrator<UIDataSourceComponent>::Begin()[M::DisplayName("Data Source"), M::Group("Data")]
    .ConstructorByPointer()
    .DestructorByPointer([](UIDataSourceComponent* o) { o->Release(); })
    .Field("dataFile", &UIDataSourceComponent::GetDataFile, &UIDataSourceComponent::SetDataFile)[M::DisplayName("Data File")]
    .End();
}

IMPLEMENT_UI_COMPONENT(UIDataSourceComponent);

UIDataSourceComponent::UIDataSourceComponent(const UIDataSourceComponent& c)
    : dataFile(c.dataFile)
    , data(c.data)
{
    isDirty = true;
}

UIDataSourceComponent* UIDataSourceComponent::Clone() const
{
    return new UIDataSourceComponent(*this);
}

const Reflection& UIDataSourceComponent::GetData() const
{
    return data;
}

void UIDataSourceComponent::SetData(const Reflection& data_)
{
    data = data_;
    isDirty = true;
}

const FilePath& UIDataSourceComponent::GetDataFile() const
{
    return dataFile;
}

void UIDataSourceComponent::SetDataFile(const FilePath& path)
{
    dataFile = path;
    isDirty = true;
}

bool UIDataSourceComponent::IsDirty() const
{
    return isDirty;
}

void UIDataSourceComponent::SetDirty(bool dirty_)
{
    isDirty = dirty_;
}
}
