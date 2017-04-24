#include "UI/Spine/UISpineComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{

DAVA_VIRTUAL_REFLECTION_IMPL(UISpineComponent)
{
    ReflectionRegistrator<UISpineComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UISpineComponent* c) { SafeRelease(c); })
    .Field("skeletonPath", &UISpineComponent::GetSkeletonPath, &UISpineComponent::SetSkeletonPath)
    .Field("atlasPath", &UISpineComponent::GetAtlasPath, &UISpineComponent::SetAtlasPath)
    .End();
}

UISpineComponent::UISpineComponent(const UISpineComponent& copy)
    : skeletonPath(copy.skeletonPath)
    , atlasPath(copy.atlasPath)
{
}

UISpineComponent* UISpineComponent::Clone() const
{
    return new UISpineComponent(*this);
}

void UISpineComponent::SetSkeletonPath(const FilePath& path)
{
    skeletonPath = path;
}

void UISpineComponent::SetAtlasPath(const FilePath& path)
{
    atlasPath = path;
}

}
