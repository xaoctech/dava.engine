#pragma once

#include "VisualScript/VisualScriptNode.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
class VisualScriptComponent;
struct CollisionInfo;

class VisualScriptEvent : public ReflectionBase
{
public:
    virtual ~VisualScriptEvent() = default;
    DAVA_VIRTUAL_REFLECTION(VisualScriptEvent, ReflectionBase);
};

class ButtonClickEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(ButtonClickEvent, VisualScriptEvent);
    ButtonClickEvent() = default;
};

class TimerEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(TimerEvent, VisualScriptEvent);
    TimerEvent() = default;
};

class EntitiesCollideEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(EntitiesCollideEvent, VisualScriptEvent);
    EntitiesCollideEvent() = default;

    CollisionInfo* collisionInfo = nullptr;
};

class ProcessEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(ProcessEvent, VisualScriptEvent);
    ProcessEvent() = default;

    VisualScriptComponent* component = nullptr;
    float32 frameDelta = 0.f;
};

class DigitalActionEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(DigitalActionEvent, VisualScriptEvent);
};

class AnalogActionEvent : public VisualScriptEvent
{
public:
    DAVA_VIRTUAL_REFLECTION(AnalogActionEvent, VisualScriptEvent);
};

} //DAVA
