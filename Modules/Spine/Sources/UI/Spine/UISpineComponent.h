#pragma once

#include <FileSystem/FilePath.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{

class UISpineComponent : public UIBaseComponent<UISpineComponent>
{
    DAVA_VIRTUAL_REFLECTION(UISpineComponent, UIBaseComponent<UISpineComponent>);

public:
    enum AnimationState
    {
        STOPPED = 0,
        PLAYED
    };

    UISpineComponent() = default;
    UISpineComponent(const UISpineComponent& copy);
    UISpineComponent& operator=(const UISpineComponent&) = delete;
    UISpineComponent* Clone() const override;

    /** Spine skeleton data file */
    const FilePath& GetSkeletonPath() const;
    void SetSkeletonPath(const FilePath& path);

    /** Spine atlas texture file */
    const FilePath& GetAtlasPath() const;
    void SetAtlasPath(const FilePath& path);

    const AnimationState& GetAnimationState() const;
    void SetAnimationState(const AnimationState& state);

    int32 GetAnimationIndex() const;
    void SetAnimationIndex(int32 index);

    String GetAnimationName() const;
    void SetAnimationName(const String& name);

    bool GetLoopedPlayback() const;
    void SetLoopedPlayback(bool looped);

    Signal<UISpineComponent* /*component*/, const String& /*name*/> onAnimationStart;
    Signal<UISpineComponent* /*component*/, const String& /*name*/> onAnimationStop;
    Signal<UISpineComponent* /*component*/, const String& /*event*/> onAnimationEvent;

protected:
    ~UISpineComponent() override = default;

private:
    /* Resources */
    FilePath skeletonPath;
    FilePath atlasPath;
    /* State */
    AnimationState animationState;
    int32 animationIndex;
    String animationName;
    bool animationLooped;
};

inline const FilePath& UISpineComponent::GetSkeletonPath() const
{
    return skeletonPath;
}

inline const FilePath& UISpineComponent::GetAtlasPath() const
{
    return atlasPath;
}

}