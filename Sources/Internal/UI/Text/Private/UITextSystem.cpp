#include "UI/Text/UITextSystem.h"

#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/Component.h"
#include "Render/2D/FontManager.h"
#include "UI/Text/UITextComponent.h"
#include "UI/UIControl.h"
#include "UITextSystemLink.h"

namespace DAVA
{
UITextSystem::~UITextSystem()
{
    for (UITextSystemLink* link : links)
    {
        // System expect that all components already removed via UnregisterComponent and UnregisterControl.
        DVASSERT(link == nullptr, "Unexpected system state! List of links must be empty!");
    }
}

void UITextSystem::Process(float32 elapsedTime)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_TEXT_SYSTEM);

    // Remove empty links
    if (!links.empty())
    {
        links.erase(std::remove(links.begin(), links.end(), nullptr), links.end());
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

void UITextSystem::ForceProcessControl(float32 elapsedTime, UIControl* control)
{
    UITextComponent* component = control->GetComponent<UITextComponent>();
    if (component)
    {
        UITextSystemLink* link = component->GetLink();
        if (link)
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

    UITextComponent* textComponent = CastIfEqual<UITextComponent*>(component);
    if (textComponent != nullptr)
    {
        AddLink(textComponent);
    }
}

void UITextSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    UITextComponent* textComponent = CastIfEqual<UITextComponent*>(component);
    if (textComponent != nullptr)
    {
        RemoveLink(textComponent);
    }

    UISystem::UnregisterComponent(control, component);
}

void UITextSystem::AddLink(UITextComponent* component)
{
    DVASSERT(component);
    DVASSERT(component->GetLink() == nullptr);
    UITextSystemLink* link = new UITextSystemLink(component->GetControl(), component);
    component->SetLink(link);
    links.push_back(link);
}

void UITextSystem::RemoveLink(UITextComponent* component)
{
    DVASSERT(component);
    UITextSystemLink* link = component->GetLink();
    DVASSERT(component->GetLink());

    auto findIt = std::find(links.begin(), links.end(), link);
    if (findIt != links.end())
    {
        (*findIt) = nullptr; // mark link for delete
        component->SetLink(nullptr);
        delete link;
    }
    else
    {
        DVASSERT(0 && "Text component link not found in system list!");
    }
}
}
