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
}

void UITextSystem::RegisterControl(UIControl* control)
{
    UISystem::RegisterControl(control);
    UIStaticTextComponent* component = control->GetComponent<UIStaticTextComponent>();
    if (component)
    {
        component->CreateDrawer();
    }
}

void UITextSystem::UnregisterControl(UIControl* control)
{
    UIStaticTextComponent* component = control->GetComponent<UIStaticTextComponent>();
    if (component)
    {
        component->DestroyDrawer();
    }

    UISystem::UnregisterControl(control);
}

void UITextSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    UISystem::RegisterComponent(control, component);

    if (component->GetType() == UIStaticTextComponent::C_TYPE)
    {
        (static_cast<UIStaticTextComponent*>(component))->CreateDrawer();
    }
}

void UITextSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == UIStaticTextComponent::C_TYPE)
    {
        (static_cast<UIStaticTextComponent*>(component))->DestroyDrawer();
    }

    UISystem::UnregisterComponent(control, component);
}
}
