#include "UI/Spine/UISpineSystem.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/SpineSkeleton.h"

#include <Render/2D/Systems/RenderSystem2D.h>
#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/Components/UIComponent.h>

#include <spine/spine.h>

namespace DAVA
{

UISpineSystem::UISpineSystem() = default;

UISpineSystem::~UISpineSystem() = default;

void UISpineSystem::RegisterControl(UIControl * control)
{
    UISpineComponent* component = control->GetComponent<UISpineComponent>();
    if (component)
    {
        AddNode(component);
    }
}

void UISpineSystem::UnregisterControl(UIControl * control)
{
    UISpineComponent* component = control->GetComponent<UISpineComponent>();
    if (component)
    {
        RemoveNode(component);
    }
}

void UISpineSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UISpineComponent>())
    {
        AddNode(static_cast<UISpineComponent*>(component));
    }
    else if (component->GetType() == Type::Instance<UIControlBackground>())
    {
        auto it = std::find_if(nodes.begin(), nodes.end(), [control](const SpineNode& node) {
            return node.component->GetControl() == control;
        });
        if (it != nodes.end())
        {
            UIControlBackground* bg = static_cast<UIControlBackground*>(component);
            bg->SetRenderBatch(it->skeleton->GetRenderBatch());
        }
    }
}

void UISpineSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UISpineComponent>())
    {
        RemoveNode(static_cast<UISpineComponent*>(component));
    }
    else if (component->GetType() == Type::Instance<UIControlBackground>())
    {
        auto it = std::find_if(nodes.begin(), nodes.end(), [control](const SpineNode& node) {
            return node.component->GetControl() == control;
        });
        if (it != nodes.end())
        {
            UIControlBackground* bg = static_cast<UIControlBackground*>(component);
            bg->SetRenderBatch(nullptr);
        }
    }
}

void UISpineSystem::OnControlVisible(UIControl * control)
{
}

void UISpineSystem::OnControlInvisible(UIControl * control)
{
}

void UISpineSystem::Process(DAVA::float32 elapsedTime)
{
    for(SpineNode& node : nodes)
    {
        UISpineComponent* component = node.component;
        const RefPtr<SpineSkeleton>& skeleton = node.skeleton;

        if (component->IsNeedReload())
        {
            skeleton->Load(component->GetSkeletonPath(), component->GetAtlasPath());

            bool modified = component->IsModified();
            component->SetAnimationsNames(skeleton->GetAvailableAnimationsNames());
            component->SetModified(modified);

            component->SetNeedReload(false);
        }

        if (component->IsModified())
        {
            switch(component->GetAnimationState())
            {
            case UISpineComponent::PLAYED:
                {
                    const String& name = component->GetAnimationName();
                    if (!name.empty())
                    {
                        if (skeleton->SetAnimation(0, name, component->IsLoopedPlayback()) == nullptr)
                        {
                            skeleton->ClearTracks();
                            skeleton->ResetSkeleton();
                        }
                    }
                }
                break;
            case UISpineComponent::STOPPED:
                skeleton->ClearTracks();
                skeleton->ResetSkeleton();
                break;
            }
            skeleton->SetTimeScale(component->GetTimeScale());
            component->SetModified(false);
        }

        skeleton->Update(elapsedTime);
    }
}

void UISpineSystem::AddNode(UISpineComponent* component)
{
    DVASSERT(component);

    SpineNode node;
    node.component = component;
    node.component->SetModified(true);
    node.component->SetNeedReload(true);

    node.skeleton.Set(new SpineSkeleton());
    node.skeleton->onStart.Connect([this, component](int32 trackIndex) {
        component->SetAnimationState(UISpineComponent::PLAYED);
        component->onAnimationStart.Emit(component, trackIndex);
        onAnimationStart.Emit(component, trackIndex);
    });
    node.skeleton->onFinish.Connect([this, component](int32 trackIndex) {
        component->SetAnimationState(UISpineComponent::STOPPED);
        component->onAnimationFinish.Emit(component, trackIndex);
        onAnimationFinish.Emit(component, trackIndex);
    });
    node.skeleton->onComplete.Connect([this, component](int32 trackIndex) {
        onAnimationComplete.Emit(component, trackIndex);
        component->onAnimationComplete.Emit(component, trackIndex);
    });
    node.skeleton->onEvent.Connect([this, component](int32 trackIndex, const String& event) {
        component->onAnimationEvent.Emit(component, trackIndex, event);
        onAnimationEvent.Emit(component, trackIndex, event);
    });

    // Bind background if exists
    UIControlBackground* bg = component->GetControl()->GetComponent<UIControlBackground>();
    if (bg)
    {
        bg->SetDrawType(UIControlBackground::DRAW_BATCH);
        bg->SetRenderBatch(node.skeleton->GetRenderBatch());
    }

    nodes.push_back(node);
}

void UISpineSystem::RemoveNode(UISpineComponent* component)
{
    DVASSERT(component);

    auto it = std::find_if(nodes.begin(), nodes.end(), [component](const SpineNode& node) {
        return node.component == component;
    });

    if (it != nodes.end())
    {
        UIControl* ctrl = it->component->GetControl();
        UIControlBackground* bg = ctrl->GetComponent<UIControlBackground>();
        if (bg)
        {
            bg->SetRenderBatch(nullptr);
        }
        nodes.erase(it, nodes.end());
    }
}

}