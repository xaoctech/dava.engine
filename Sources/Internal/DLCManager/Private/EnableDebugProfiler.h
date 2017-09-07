#pragma once

#include "Functional/Signal.h"
#include "Input/InputEvent.h"
#include "Debug/Private/RingArray.h"

namespace DAVA
{
class Window;

class EnableDebugProfiler
{
public:
    EnableDebugProfiler();
    ~EnableDebugProfiler();
    void AddListenerOnMouseAndTouch();
    Signal<bool> debugGesture;

private:
    void OnWindowCreated(Window* window);
    bool OnMouseOrTouch(const InputEvent& ev);
    RingArray<InputEvent> history;
    uint32 handlerToken = 0;
    bool isDebugEnabled = false;
};
}
