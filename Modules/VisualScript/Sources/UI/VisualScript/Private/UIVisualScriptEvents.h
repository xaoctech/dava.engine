#pragma once
#include "VisualScript/VisualScriptEvents.h"

#include "UI/VisualScript/UIVisualScriptComponent.h"

namespace DAVA
{
// TODO: rename with `VS` or use namespace

class UIInitEvent : public VisualScriptEvent
{
    DAVA_VIRTUAL_REFLECTION(UIInitEvent, VisualScriptEvent);

public:
    UIVisualScriptComponent* component = nullptr;
    static const FastName NAME;
};

class UIReleaseEvent : public VisualScriptEvent
{
    DAVA_VIRTUAL_REFLECTION(UIInitEvent, VisualScriptEvent);

public:
    UIVisualScriptComponent* component = nullptr;
    static const FastName NAME;
};

class UIProcessEvent : public VisualScriptEvent
{
    DAVA_VIRTUAL_REFLECTION(UIProcessEvent, VisualScriptEvent);

public:
    UIVisualScriptComponent* component = nullptr;
    float32 frameDelta = 0.f;
    static const FastName NAME;
};

class UIEventProcessEvent : public VisualScriptEvent
{
    DAVA_VIRTUAL_REFLECTION(UIEventProcessEvent, VisualScriptEvent);

public:
    UIVisualScriptComponent* component = nullptr;
    FastName eventName;
    static const FastName NAME;
};
} // namespace DAVA
