#include "UITextSystem.h"

#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "UI/UIControl.h"
#include "UIStaticTextComponent.h"
#include "UITextSystemLink.h"

namespace DAVA
{
void UITextSystem::Process(float32 elapsedTime)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_TEXT_SYSTEM);

    // Remove empty links
    if (!links.empty())
    {
        links.erase(std::remove_if(links.begin(), links.end(), [](const UITextSystemLink* l) {
                        return l == nullptr;
                    }),
                    links.end());
    }

    // Process links
    for (UITextSystemLink* link : links)
    {
        if (link != nullptr && link->component->IsModified())
        {
            link->ApplyData();
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
    UITextSystemLink* link = new UITextSystemLink(component->GetControl(), component);
    component->SetLink(link);
    links.emplace_back(link);
}

void UITextSystem::RemoveLink(UIStaticTextComponent* component)
{
    DVASSERT(component);
    UITextSystemLink* link = component->GetLink();
    for (UITextSystemLink*& l : links)
    {
        if (l == link)
        {
            l = nullptr;
            break;
        }
    }
    auto findIt = std::find_if(links.begin(), links.end(), [&link](const UITextSystemLink* l) {
        //return l != nullptr && l->component == component;
        return l == link;
    });
    if (findIt != links.end())
    {
        (*findIt) = nullptr; // mark link for delete
    }
    component->SetLink(nullptr);
    delete link;
}

}
