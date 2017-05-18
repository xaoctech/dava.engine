#include "UI/Spine/UISpineSystem.h"

#include "UI/Spine/UISpineBonesComponent.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/SpineSkeleton.h"

#include <Render/2D/Systems/RenderSystem2D.h>
#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/Components/UIComponent.h>
#include <UI/Layouts/UILayoutSourceRectComponent.h>

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
    else if (component->GetType() == Type::Instance<UISpineBonesComponent>())
    {
        BindBones(static_cast<UISpineBonesComponent*>(component));
    }
    else if (component->GetType() == Type::Instance<UIControlBackground>())
    {
        BindBackground(static_cast<UIControlBackground*>(component));
    }
}

void UISpineSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UISpineComponent>())
    {
        RemoveNode(static_cast<UISpineComponent*>(component));
    }
    else if (component->GetType() == Type::Instance<UISpineBonesComponent>())
    {
        UnbindBones(static_cast<UISpineBonesComponent*>(component));
    }
    else if (component->GetType() == Type::Instance<UIControlBackground>())
    {
        UnbindBackground(static_cast<UIControlBackground*>(component));
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
    for(auto& pair : nodes)
    {
        SpineNode& node = pair.second;
        RefPtr<UISpineComponent>& spine = node.spine;
        RefPtr<UISpineBonesComponent>& bones = node.bones;
        const RefPtr<SpineSkeleton>& skeleton = node.skeleton;

        if (spine->IsNeedReload())
        {
            skeleton->Load(spine->GetSkeletonPath(), spine->GetAtlasPath());

            spine->SetAnimationsNames(skeleton->GetAvailableAnimationsNames());
            spine->SetSkinsNames(skeleton->GetAvailableSkinsNames());
            
            spine->SetNeedReload(false);
        }

        if (spine->IsModified())
        {
            skeleton->SetTimeScale(spine->GetTimeScale());
            skeleton->SetSkin(spine->GetSkinName());
            switch(spine->GetAnimationState())
            {
            case UISpineComponent::PLAYED:
                {
                    const String& name = spine->GetAnimationName();
                    if (name.empty() || skeleton->SetAnimation(0, name, spine->IsLoopedPlayback()) == nullptr)
                    {
                        skeleton->ClearTracks();
                        skeleton->ResetSkeleton();
                    }
                }
                break;
            case UISpineComponent::STOPPED:
                skeleton->ClearTracks();
                skeleton->ResetSkeleton();
                break;
            }
            spine->SetModified(false);
        }

        skeleton->Update(elapsedTime);

        if (bones)
        {
            for (auto& bonePair : bones->GetBinds())
            {
                spBone* bone = reinterpret_cast<spBone*>(skeleton->FindBone(bonePair.first));
                //DVASSERT(bone != nullptr, "Bone was not found!");
                if (bone)
                {
                    UIControl* boneControl = pair.first->FindByPath(bonePair.second);
                    //DVASSERT(boneControl != nullptr, "Control was not found!");
                    if (boneControl)
                    {
                        UILayoutSourceRectComponent* lsrc = boneControl->GetComponent<UILayoutSourceRectComponent>();
                        if (lsrc)
                        {
                            lsrc->SetPosition(Vector2(bone->worldX, -bone->worldY));
                        }

                        boneControl->SetPosition(Vector2(bone->worldX, -bone->worldY));
                        boneControl->SetAngleInDegrees(-bone->rotation);
                        boneControl->SetScale(Vector2(bone->scaleX, bone->scaleY));
                        //boneControl->UpdateLayout();
                    }
                }
            }
        }
    }
}

void UISpineSystem::AddNode(UISpineComponent* component)
{
    DVASSERT(component);

    SpineNode node;
    node.spine = component;
    node.spine->SetModified(true);
    node.spine->SetNeedReload(true);

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

    nodes[component->GetControl()] = node;

    // Bind background if exists
    UIControlBackground* bg = component->GetControl()->GetComponent<UIControlBackground>();
    if (bg)
    {
        BindBackground(bg);
    }

    // Bind bones if exists
    UISpineBonesComponent* bones = component->GetControl()->GetComponent<UISpineBonesComponent>();
    if (bones)
    {
        BindBones(bones);
    }
}

void UISpineSystem::RemoveNode(UISpineComponent* component)
{
    DVASSERT(component);

    auto it = nodes.find(component->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;

        if (node.bg)
        {
            node.bg->SetRenderBatch(nullptr);
        }

        if (node.bones)
        {

        }

        nodes.erase(it, nodes.end());
    }
}

void UISpineSystem::BindBones(UISpineBonesComponent * bones)
{
    auto it = nodes.find(bones->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;
        DVASSERT(node.bones == nullptr);
        node.bones = bones;
    }
}

void UISpineSystem::UnbindBones(UISpineBonesComponent * bones)
{
    auto it = nodes.find(bones->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;
        DVASSERT(node.bones == bones);
        node.bones.Set(nullptr);
    }
}

void UISpineSystem::BindBackground(UIControlBackground * bg)
{
    auto it = nodes.find(bg->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;
        DVASSERT(node.bg == nullptr);
        node.bg = bg;
        node.bg->SetRenderBatch(node.skeleton->GetRenderBatch());
    }
}

void UISpineSystem::UnbindBackground(UIControlBackground * bg)
{
    auto it = nodes.find(bg->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;
        DVASSERT(node.bg == bg);
        node.bg->SetRenderBatch(nullptr);
        node.bg.Set(nullptr);
    }
}

}