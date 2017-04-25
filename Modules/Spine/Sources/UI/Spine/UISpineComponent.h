#pragma once

#include <FileSystem/FilePath.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{

class UISpineComponent : public UIBaseComponent<UISpineComponent>
{
    DAVA_VIRTUAL_REFLECTION(UISpineComponent, UIComponent);

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

    const String& GetAnimationName() const;
    void SetAnimationName(const String& name);

    const Vector<String>& GetAnimationsNames() const;
    void SetAnimationsNames(const Vector<String>& names);

    bool IsLoopedPlayback() const;
    void SetLoopedPlayback(bool loop);

    bool IsModified() const;
    void SetModified(bool modify);

    bool IsNeedReload() const;
    void SetNeedReload(bool reload);

    Signal<UISpineComponent* /*component*/, const String& /*name*/> onAnimationStart;
    Signal<UISpineComponent* /*component*/, const String& /*name*/> onAnimationStop;
    Signal<UISpineComponent* /*component*/, const String& /*event*/> onAnimationEvent;

protected:
    ~UISpineComponent() override;

private:
    /* Resources */
    FilePath skeletonPath;
    FilePath atlasPath;

    /* State */
    String animationName;
    Vector<String> animationsNames;
    AnimationState animationState = STOPPED;
    bool needReload = false;
    bool modified = false;
    bool animationLooped = false;
};

inline const FilePath& UISpineComponent::GetSkeletonPath() const
{
    return skeletonPath;
}

inline const FilePath& UISpineComponent::GetAtlasPath() const
{
    return atlasPath;
}

inline const UISpineComponent::AnimationState& UISpineComponent::GetAnimationState() const
{
    return animationState;
}

inline const String& UISpineComponent::GetAnimationName() const
{
    return animationName;
}

inline const Vector<String>& UISpineComponent::GetAnimationsNames() const
{
    return animationsNames;
}

inline bool UISpineComponent::IsLoopedPlayback() const
{
    return animationLooped;
}

inline bool UISpineComponent::IsModified() const
{
    return modified;
}

inline bool UISpineComponent::IsNeedReload() const
{
    return needReload;
}

}