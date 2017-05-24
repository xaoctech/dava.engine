#include "UI/Spine/UISpineSystem.h"

#include "UI/Spine/UISpineBonesComponent.h"
#include "UI/Spine/UISpineComponent.h"
#include "UI/Spine/SpineSkeleton.h"

#include <Render/2D/Systems/RenderSystem2D.h>
#include <UI/Components/UIComponent.h>
#include <UI/Layouts/UILayoutSourceRectComponent.h>
#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/UIControlSystem.h>

#include <spine/spine.h>

namespace DAVA
{
UISpineSystem::UISpineSystem() = default;

UISpineSystem::~UISpineSystem() = default;

void UISpineSystem::RegisterControl(UIControl* control)
{
    UISpineComponent* component = control->GetComponent<UISpineComponent>();
    if (component)
    {
        AddNode(component);
    }
}

void UISpineSystem::UnregisterControl(UIControl* control)
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

void UISpineSystem::OnControlVisible(UIControl* control)
{
}

void UISpineSystem::OnControlInvisible(UIControl* control)
{
}

void UISpineSystem::Process(DAVA::float32 elapsedTime)
{
    UISpineSingleComponent* spineSingle = GetScene()->GetSingleComponent<UISpineSingleComponent>();

    for (UIControl* c : spineSingle->spineNeedReload)
    {
        auto it = nodes.find(c);
        if (it != nodes.end())
        {
            SpineNode& node = it->second;

            node.skeleton->Load(node.spine->GetSkeletonPath(), node.spine->GetAtlasPath());

            node.spine->SetAnimationsNames(node.skeleton->GetAvailableAnimationsNames());
            node.spine->SetSkinsNames(node.skeleton->GetAvailableSkinsNames());

            // Build bones links after load skeleton
            spineSingle->spineBonesModified.insert(c);
        }
    }

    for (UIControl* c : spineSingle->spineModified)
    {
        auto it = nodes.find(c);
        if (it != nodes.end())
        {
            SpineNode& node = it->second;

            node.skeleton->SetTimeScale(node.spine->GetTimeScale());
            node.skeleton->SetSkin(node.spine->GetSkinName());
            switch (node.spine->GetAnimationState())
            {
            case UISpineComponent::PLAYED:
            {
                const String& name = node.spine->GetAnimationName();
                if (name.empty() || node.skeleton->SetAnimation(0, name, node.spine->IsLoopedPlayback()) == nullptr)
                {
                    node.skeleton->ClearTracks();
                    node.skeleton->ResetSkeleton();
                }
            }
            break;
            case UISpineComponent::STOPPED:
                node.skeleton->ClearTracks();
                node.skeleton->ResetSkeleton();
                break;
            }
        }
    }

    for (UIControl* c : spineSingle->spineBonesModified)
    {
        auto it = nodes.find(c);
        if (it != nodes.end())
        {
            SpineNode& node = it->second;
            if (node.bones)
            {
                BuildBoneLinks(node);
            }
            else if (!node.boneLinks.empty())
            {
                node.boneLinks.clear();
            }
        }
    }

    for (auto& pair : nodes)
    {
        SpineNode& node = pair.second;
        const RefPtr<UISpineComponent>& spine = node.spine;
        const RefPtr<SpineSkeleton>& skeleton = node.skeleton;

        skeleton->Update(elapsedTime);

        for (const BoneLink& link : node.boneLinks)
        {
            // TODO: Discuss with d_belskiy other workflow with UILayoutSourceRectComponent
            UILayoutSourceRectComponent* lsrc = link.control->GetComponent<UILayoutSourceRectComponent>();
            if (lsrc)
            {
                lsrc->SetPosition(link.bone->GetPosition());
            }

            link.control->SetPosition(link.bone->GetPosition());
            link.control->SetAngleInDegrees(link.bone->GetAngle());
            link.control->SetScale(link.bone->GetScale());
            link.control->UpdateLayout();
        }
    }
}

void UISpineSystem::AddNode(UISpineComponent* component)
{
    DVASSERT(component);

    SpineNode node;
    node.spine = component;
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

    UISpineSingleComponent* spineSingle = GetScene()->GetSingleComponent<UISpineSingleComponent>();
    spineSingle->spineModified.insert(component->GetControl());
    spineSingle->spineNeedReload.insert(component->GetControl());

    // Bind bones if exists
    UISpineBonesComponent* bones = component->GetControl()->GetComponent<UISpineBonesComponent>();
    if (bones)
    {
        BindBones(bones);
    }

    // Bind background if exists
    UIControlBackground* bg = component->GetControl()->GetComponent<UIControlBackground>();
    if (bg)
    {
        BindBackground(bg);
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
            node.boneLinks.clear();
        }

        nodes.erase(it, nodes.end());
    }
}

void UISpineSystem::BindBones(UISpineBonesComponent* bones)
{
    auto it = nodes.find(bones->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;
        DVASSERT(node.bones == nullptr);
        node.bones = bones;

        UISpineSingleComponent* spineSingle = GetScene()->GetSingleComponent<UISpineSingleComponent>();
        spineSingle->spineBonesModified.insert(bones->GetControl());
    }
}

void UISpineSystem::UnbindBones(UISpineBonesComponent* bones)
{
    auto it = nodes.find(bones->GetControl());
    if (it != nodes.end())
    {
        SpineNode& node = it->second;
        DVASSERT(node.bones == bones);
        node.bones.Set(nullptr);

        UISpineSingleComponent* spineSingle = GetScene()->GetSingleComponent<UISpineSingleComponent>();
        spineSingle->spineBonesModified.insert(bones->GetControl());
    }
}

void UISpineSystem::BindBackground(UIControlBackground* bg)
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

void UISpineSystem::UnbindBackground(UIControlBackground* bg)
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

void UISpineSystem::BuildBoneLinks(SpineNode& node)
{
    node.boneLinks.clear();
    if (node.bones && node.skeleton)
    {
        UIControl* root = node.bones->GetControl();
        for (auto& bonePair : node.bones->GetBinds())
        {
            RefPtr<SpineBone> bone = node.skeleton->FindBone(bonePair.first);
            if (bone)
            {
                UIControl* boneControl = root->FindByPath(bonePair.second);
                if (boneControl)
                {
                    BoneLink link;
                    link.bone = bone;
                    link.control = boneControl;
                    node.boneLinks.push_back(std::move(link));
                }
            }
        }
    }
}

void UISpineSingleComponent::Clear()
{
    spineModified.clear();
    spineNeedReload.clear();
    spineBonesModified.clear();
}
}