#include "UI/Spine/UISpineComponent.h"

#include "UI/Spine/UISpineSingleComponent.h"
#include "UI/Spine/UISpineSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <UI/UIControlSystem.h>

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
    : UIComponent(copy)
    , skeletonPath(copy.skeletonPath)
    , atlasPath(copy.atlasPath)
    , animationState(copy.animationState)
    , animationName(copy.animationName)
    , animationsNames(copy.animationsNames)
    , animationLooped(copy.animationLooped)
{
}

UISpineComponent::~UISpineComponent() = default;

void UISpineComponent::Modify(bool needReload)
{
    UIControl* control = GetControl();
    if (control)
    {
        UIControlSystem* scene = control->GetScene();
        if (scene)
        {
            UISpineSingleComponent* spineSingle = scene->GetSingleComponent<UISpineSingleComponent>();
            if (spineSingle)
            {
                spineSingle->spineModified.insert(control);
                if (needReload)
                {
                    spineSingle->spineNeedReload.insert(control);
                }
            }
        }
    }
}

UISpineComponent* UISpineComponent::Clone() const
{
    return new UISpineComponent(*this);
}

void UISpineComponent::SetSkeletonPath(const FilePath& path)
{
    if (skeletonPath != path)
    {
        skeletonPath = path;
        Modify(true);
    }
}

void UISpineComponent::SetAtlasPath(const FilePath& path)
{
    if (atlasPath != path)
    {
        atlasPath = path;
        Modify(true);
    }
}

void UISpineComponent::SetAnimationState(const AnimationState& state)
{
    if (animationState != state)
    {
        animationState = state;
        Modify(false);
    }
}

void UISpineComponent::SetAnimationName(const String& name)
{
    if (animationName != name)
    {
        animationName = name;
        Modify(false);
    }
}

void UISpineComponent::SetAnimationsNames(const Vector<String>& names)
{
    if (animationsNames != names)
    {
        animationsNames = names;
        Modify(false);
    }
}

void UISpineComponent::SetTimeScale(float32 scale)
{
    if (timeScale != scale)
    {
        timeScale = scale;
        Modify(false);
    }
}

void UISpineComponent::SetLoopedPlayback(bool loop)
{
    if (animationLooped != loop)
    {
        animationLooped = loop;
        Modify(false);
    }
}

void UISpineComponent::SetSkinName(const String& name)
{
    if (skinName != name)
    {
        skinName = name;
        Modify(false);
    }
}

void UISpineComponent::SetSkinsNames(const Vector<String>& names)
{
    if (skinsNames != names)
    {
        skinsNames = names;
        Modify(false);
    }
}
}
