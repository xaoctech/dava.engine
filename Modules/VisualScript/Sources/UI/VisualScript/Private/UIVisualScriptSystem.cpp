#include "UI/VisualScript/UIVisualScriptSystem.h"
#include "VisualScript/VisualScript.h"
#include "UI/VisualScript/Private/UIVisualScriptEvents.h"
#include "UI/VisualScript/UIVisualScriptComponent.h"

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>
#include <UI/Events/UIEventsSystem.h>

namespace DAVA
{
namespace UIVisualScriptSystemDetails
{
const FastName EVENT_PROCESS_RESULT_KEY("processEventResult");
}

UIVisualScriptSystem::~UIVisualScriptSystem() = default;

void UIVisualScriptSystem::RegisterSystem()
{
    if (GetScene())
    {
        UIEventsSystem* eSys = GetScene()->GetSystem<UIEventsSystem>();
        DVASSERT(eSys); // Depends on UIEventsSystem
        eSys->AddProcessor(this);
    }
}

void UIVisualScriptSystem::RegisterControl(UIControl* control)
{
    UIVisualScriptComponent* component = control->GetComponent<UIVisualScriptComponent>();
    if (component)
    {
        AddLink(component);
    }
}

void UIVisualScriptSystem::UnregisterControl(UIControl* control)
{
    UIVisualScriptComponent* component = control->GetComponent<UIVisualScriptComponent>();
    if (component)
    {
        RemoveLink(component);
    }
}

void UIVisualScriptSystem::RegisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIVisualScriptComponent>())
    {
        AddLink(static_cast<UIVisualScriptComponent*>(component));
    }
}

void UIVisualScriptSystem::UnregisterComponent(UIControl* control, UIComponent* component)
{
    if (component->GetType() == Type::Instance<UIVisualScriptComponent>())
    {
        RemoveLink(static_cast<UIVisualScriptComponent*>(component));
    }
}

void UIVisualScriptSystem::Process(float32 elapsedTime)
{
    for (auto& pair : links)
    {
        UIVisualScriptComponent* component = pair.first;
        Link& l = pair.second;

        if (!component->IsEnabled())
        {
            continue;
        }

        if (component->IsNeedReload())
        {
            if (l.ready)
            {
                UIReleaseEvent event;
                event.component = component;
                try
                {
                    l.script->Execute(event.NAME, Reflection::Create(&event));
                }
                catch (Exception& e)
                {
                    Logger::Error(e.what());
                }
                l.ready = false;
                l.script.reset();
            }

            const FilePath& scriptPath = component->GetScriptPath();
            if (GetEngineContext()->fileSystem->Exists(scriptPath) == true)
            {
                l.script = std::make_shared<VisualScript>();
                try
                {
                    l.script->Load(scriptPath);
                    l.script->Compile();
                    l.ready = true;

                    UpdateVariables(component, l);

                    UIInitEvent event;
                    event.component = component;
                    try
                    {
                        l.script->Execute(event.NAME, Reflection::Create(&event));
                    }
                    catch (Exception& e)
                    {
                        Logger::Error(e.what());
                    }
                }
                catch (Exception& e)
                {
                    l.ready = false;
                    l.script.reset();
                    Logger::Error(e.what());
                }
            }
            else
            {
                Logger::Warning("VisualScript file '%s' does not exist !.", scriptPath.GetStringValue().c_str());
            }
            component->SetNeedReload(false);
        }

        if (l.ready)
        {
            UIProcessEvent event;
            event.component = component;
            event.frameDelta = elapsedTime;
            try
            {
                l.script->Execute(event.NAME, Reflection::Create(&event));
            }
            catch (Exception& e)
            {
                Logger::Error(e.what());
            }
        }
    }
}

bool UIVisualScriptSystem::ProcessEvent(UIControl* control, const FastName& eventName)
{
    UIVisualScriptComponent* component = control->GetComponent<UIVisualScriptComponent>();
    if (component && component->IsEnabled())
    {
        auto it = links.find(component);
        if (it != links.end())
        {
            Link& l = it->second;
            if (l.ready)
            {
                UIEventProcessEvent event;
                event.component = component;
                event.eventName = eventName;
                try
                {
                    l.script->Execute(event.NAME, Reflection::Create(&event));
                }
                catch (Exception& e)
                {
                    Logger::Error(e.what());
                }
            }
        }
    }
    return false;
}

void UIVisualScriptSystem::AddLink(UIVisualScriptComponent* c)
{
    links[c] = Link{};
}

void UIVisualScriptSystem::RemoveLink(UIVisualScriptComponent* c)
{
    auto it = links.find(c);
    if (it != links.end())
    {
        Link& l = it->second;
        if (l.ready)
        {
            UIReleaseEvent event;
            event.component = c;
            try
            {
                l.script->Execute(event.NAME, Reflection::Create(&event));
            }
            catch (Exception& e)
            {
                Logger::Error(e.what());
            }
        }
        links.erase(it);
    }
}
void UIVisualScriptSystem::UpdateVariables(UIVisualScriptComponent* component, Link& link)
{
    DVASSERT(link.ready);
    VarTable& varTable = component->GetProperties();
    bool updateDefaults = !varTable.HasDefaultValues();
    for (auto& pair : link.script->dataRegistry)
    {
        FastName propertyName = pair.first;
        Any value = pair.second;
        if (updateDefaults)
        {
            varTable.SetDefaultValue(propertyName, value);
        }

        if (!varTable.HasProperty(propertyName))
        {
            varTable.SetPropertyValue(propertyName, value);
        }
        else
        {
            if (value.GetType() != varTable.GetPropertyValue(propertyName).GetType())
            {
                Logger::Error("Script property '%s' has invalid type.", propertyName.c_str());
                varTable.SetPropertyValue(propertyName, value);
            }
            else
            {
                pair.second = varTable.GetPropertyValue(propertyName);
            }
        }
    }
}
}
