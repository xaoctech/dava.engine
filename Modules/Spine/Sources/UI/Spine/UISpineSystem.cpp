#include "UISpineSystem.h"
#include "spine/spine.h"

namespace DAVA
{

void UISpineSystem::RegisterControl(UIControl * control)
{
}

void UISpineSystem::UnregisterControl(UIControl * control)
{
}

void UISpineSystem::RegisterComponent(UIControl * control, UIComponent * component)
{
}

void UISpineSystem::UnregisterComponent(UIControl * control, UIComponent * component)
{
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
        SpineSkeleton* skeleton = node.skeleton;

        skeleton->Update(elapsedTime);

        UIControl* control = component->GetControl();
        UIControlBackground* bg = control->GetComponent<UIControlBackground>();
        if (bg)
        {
            bg->SetRenderBatch(skeleton->GetRednerBatch());
        }
    }
}

void UISpineSystem::AddNode(UISpineComponent* component)
{
    SpineNode node;
    node.component = component;
    node.skeleton = new SpineSkeleton();
    node.skeleton.Load(component->GetSkeletonPath(), component->GetAtlasPath());
    node.skeleton.onEvent([this, component](const String& event) {
        onAnimationEvent.Emit(component, event);
    });
    nodes.push_back(node);
}

void UISpineSystem::RemoveNode(UISpineComponent* component)
{
}

}