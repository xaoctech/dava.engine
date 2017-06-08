#include "UITextSystem.h"

#include "UIStaticTextComponent.h"

#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "Render/Renderer.h"
#include "UI/UIControl.h"
#include "UI/UIScreen.h"

namespace DAVA
{
void UITextSystem::Process(float32 elapsedTime)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_TEXT_SYSTEM);

    // Remove empty links
    if (!links.empty())
    {
        links.erase(std::remove_if(links.begin(), links.end(), [](const Link& l) {
                        return l.component == nullptr;
                    }),
                    links.end());
    }

    // Process links
    for (Link& l : links)
    {
        if (l.component && l.component->IsModified())
        {
            UIStaticTextState* state = l.component->GetState();
            DVASSERT(state);
            state->ApplyComponentData();
            l.component->SetModified(false);
        }
    }
}

void UITextSystem::RegisterControl(UIControl* control)
{
    UISystem::RegisterControl(control);
    UIStaticTextComponent* component = control->GetComponent<UIStaticTextComponent>();
    if (component)
    {
        AddLink(component);
    }
}

void UITextSystem::UnregisterControl(UIControl* control)
{
    UIStaticTextComponent* component = control->GetComponent<UIStaticTextComponent>();
    if (component)
    {
        RemoveLink(component);
    }

    UISystem::UnregisterControl(control);
}

void UITextSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    UISystem::RegisterComponent(control, component);

    if (component->GetType() == UIStaticTextComponent::C_TYPE)
    {
        AddLink(static_cast<UIStaticTextComponent*>(component));
    }
}

void UITextSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIStaticTextComponent::C_TYPE)
    {
        RemoveLink(static_cast<UIStaticTextComponent*>(component));
    }

    UISystem::UnregisterComponent(control, component);
}

void UITextSystem::AddLink(UIStaticTextComponent* component)
{
    DVASSERT(component);
    UIStaticTextState* state = new UIStaticTextState(component->GetControl(), component);
    component->SetState(state);
    links.emplace_back(component);
}

void UITextSystem::RemoveLink(UIStaticTextComponent* component)
{
    DVASSERT(component);
    auto findIt = std::find_if(links.begin(), links.end(), [&component](const Link& l) {
        return l.component == component;
    });
    if (findIt != links.end())
    {
        findIt->component = nullptr; // mark link for delete
    }
    UIStaticTextState* state = component->GetState();
    component->SetState(nullptr);
    SafeRelease(state);
}

UITextSystem::Link::Link(UIStaticTextComponent* c)
    : component(c)
{
}
}
