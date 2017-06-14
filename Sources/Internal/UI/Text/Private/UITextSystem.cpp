#include "UI/Text/UITextSystem.h"

#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "UI/Text/UITextComponent.h"
#include "UI/UIControl.h"
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
    UITextComponent* component = control->GetComponent<UITextComponent>();
    if (component)
    {
        AddLink(component);
    }
}

void UITextSystem::UnregisterControl(UIControl* control)
{
    UITextComponent* component = control->GetComponent<UITextComponent>();
    if (component)
    {
        RemoveLink(component);
    }

    UISystem::UnregisterControl(control);
}

void UITextSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    UISystem::RegisterComponent(control, component);

    if (component->GetType() == UITextComponent::C_TYPE)
    {
        AddLink(DAVA::DynamicTypeCheck<UITextComponent*>(component));
    }
}

void UITextSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UITextComponent::C_TYPE)
    {
        RemoveLink(DAVA::DynamicTypeCheck<UITextComponent*>(component));
    }

    UISystem::UnregisterComponent(control, component);
}

void UITextSystem::AddLink(UITextComponent* component)
{
    DVASSERT(component);
    DVASSERT(component->GetLink() == nullptr);
    UITextSystemLink* link = new UITextSystemLink(component->GetControl(), component);
    component->SetLink(link);
    links.emplace_back(link);
}

void UITextSystem::RemoveLink(UITextComponent* component)
{
    DVASSERT(component);
    UITextSystemLink* link = component->GetLink();
    DVASSERT(component->GetLink());

    auto findIt = std::find_if(links.begin(), links.end(), [&link](const UITextSystemLink* l) {
        return l == link;
    });
    if (findIt != links.end())
    {
        (*findIt) = nullptr; // mark link for delete
        component->SetLink(nullptr);
        delete link;
    }
    else
    {
        DVASSERT("Text component link not found in system list!");
    }
}
}
