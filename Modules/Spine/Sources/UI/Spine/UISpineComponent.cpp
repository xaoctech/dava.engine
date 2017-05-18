#include "UI/Spine/UISpineComponent.h"

#include <Reflection/ReflectionRegistrator.h>

ENUM_DECLARE(DAVA::UISpineComponent::AnimationState)
{
    ENUM_ADD_DESCR(DAVA::UISpineComponent::STOPPED, "Stopped");
    ENUM_ADD_DESCR(DAVA::UISpineComponent::PLAYED, "Played");
}

namespace DAVA
{

DAVA_VIRTUAL_REFLECTION_IMPL(UISpineComponent)
{
    ReflectionRegistrator<UISpineComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UISpineComponent* c) { SafeRelease(c); })
    .Field("skeletonPath", &UISpineComponent::GetSkeletonPath, &UISpineComponent::SetSkeletonPath)
    .Field("atlasPath", &UISpineComponent::GetAtlasPath, &UISpineComponent::SetAtlasPath)
    .Field("animationName", &UISpineComponent::GetAnimationName, &UISpineComponent::SetAnimationName) // Connect select to animationsNames
    .Field("animationsNames", &UISpineComponent::GetAnimationsNames, &UISpineComponent::SetAnimationsNames)[M::ReadOnly()]
    .Field("animationState", &UISpineComponent::GetAnimationState, &UISpineComponent::SetAnimationState)[M::EnumT<UISpineComponent::AnimationState>()]
    .Field("skinName", &UISpineComponent::GetSkinName, &UISpineComponent::SetSkinName) // Connect select to skinsNames
    .Field("skinsNames", &UISpineComponent::GetSkinsNames, &UISpineComponent::SetSkinsNames)[M::ReadOnly()]
    .Field("timeScale", &UISpineComponent::GetTimeScale, &UISpineComponent::SetTimeScale)
    .Field("loopedPlayback", &UISpineComponent::IsLoopedPlayback, &UISpineComponent::SetLoopedPlayback)
    .End();
}

UISpineComponent::UISpineComponent(const UISpineComponent& copy)
    : skeletonPath(copy.skeletonPath)
    , atlasPath(copy.atlasPath)
    , animationState(copy.animationState)
    , animationName(copy.animationName)
    , animationsNames(copy.animationsNames)
    , animationLooped(copy.animationLooped)
    , needReload(true)
    , modified(true)
{
}

UISpineComponent::~UISpineComponent() = default;

UISpineComponent* UISpineComponent::Clone() const
{
    return new UISpineComponent(*this);
}

void UISpineComponent::SetSkeletonPath(const FilePath& path)
{
    if (skeletonPath != path)
    {
        skeletonPath = path;
        needReload = true;
        modified = true;
    }
}

void UISpineComponent::SetAtlasPath(const FilePath& path)
{
    if (atlasPath != path)
    {
        atlasPath = path;
        needReload = true;
        modified = true;
    }
}

void UISpineComponent::SetAnimationState(const AnimationState& state)
{
    if (animationState != state)
    {
        animationState = state;
        modified = true;
    }
}

void UISpineComponent::SetAnimationName(const String& name)
{
    if (animationName != name)
    {
        animationName = name;
        modified = true;
    }
}

void UISpineComponent::SetAnimationsNames(const Vector<String>& names)
{
    if (animationsNames != names)
    {
        animationsNames = names;
        modified = true;
    }
}

void UISpineComponent::SetTimeScale(float32 scale)
{
    if (timeScale != scale)
    {
        timeScale = scale;
        modified = true;
    }
}

void UISpineComponent::SetLoopedPlayback(bool loop)
{
    if (animationLooped != loop)
    {
        animationLooped = loop;
        modified = true;
    }
}

void UISpineComponent::SetModified(bool modify)
{
    modified = modify;
}

void UISpineComponent::SetNeedReload(bool reload)
{
    needReload = reload;
}

void UISpineComponent::SetSkinName(const String& name)
{
    if (skinName != name)
    {
        skinName = name;
        modified = true;
    }
}

void UISpineComponent::SetSkinsNames(const Vector<String>& names)
{
    if (skinsNames != names)
    {
        skinsNames = names;
        modified = true;
    }
}

}
