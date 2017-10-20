#pragma once

#include "Reflection/Reflection.h"

namespace DAVA
{
class UIContext;
class UIControl;
class UIScriptComponent;

class UIScriptComponentController : public ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION(UIScriptComponentController, ReflectionBase);

public:
    virtual ~UIScriptComponentController();

    virtual void Init(UIScriptComponent* component)
    {
    }
    virtual void Release(UIScriptComponent* component)
    {
    }
    virtual void ParametersChanged(UIScriptComponent* component)
    {
    }
    virtual void Process(UIScriptComponent* component, float32 elapsedTime)
    {
    }
    virtual bool ProcessEvent(UIScriptComponent* component, const FastName& eventName, const Vector<Any>& params = Vector<Any>())
    {
        return false;
    }
};
}