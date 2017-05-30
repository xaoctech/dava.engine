#pragma once

#include <FileSystem/FilePath.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{
class UISpineComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UISpineComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UISpineComponent);

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

    const String& GetSkinName() const;
    void SetSkinName(const String& name);

    const Vector<String>& GetSkinsNames() const;
    void SetSkinsNames(const Vector<String>& names);

    float32 GetTimeScale() const;
    void SetTimeScale(float32 scale);

    bool IsLoopedPlayback() const;
    void SetLoopedPlayback(bool loop);

    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationStart;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationFinish;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationComplete;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/, const String& /*event*/> onAnimationEvent;

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
    String skinName;
    Vector<String> skinsNames;
    float32 timeScale = 1.f;
    bool animationLooped = false;

    void Modify(bool needReload = false);
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

inline const String& UISpineComponent::GetSkinName() const
{
    return skinName;
}

inline const Vector<String>& UISpineComponent::GetSkinsNames() const
{
    return skinsNames;
}

inline bool UISpineComponent::IsLoopedPlayback() const
{
    return animationLooped;
}

inline float32 UISpineComponent::GetTimeScale() const
{
    return timeScale;
}
}