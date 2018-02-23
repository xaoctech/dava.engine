#pragma once
#include "VisualScript/VisualScriptNode.h"

#include <Reflection/Reflection.h>
#include <Scene3D/Components/SingleComponents/CollisionSingleComponent.h>

namespace DAVA
{
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
