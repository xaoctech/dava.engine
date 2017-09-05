#pragma once

#include "EditorSystems/EditorSystemsManager.h"

namespace DAVA
{
class UIEvent;
namespace TArc
{
class ContextAccessor;
}
}

using CanvasControls = DAVA::Map<DAVA::uint32, DAVA::RefPtr<DAVA::UIControl>>;

class BaseEditorSystem
{
public:
    BaseEditorSystem(DAVA::TArc::ContextAccessor* accessor);

    virtual ~BaseEditorSystem() = default;

protected:
    //A client module can declare one or more own types and use them later, but can not use any other values
    enum eCanvas
    {
        DISPLAY_CANVAS,
        GRID_CANVAS,
        HUD_CANVAS
    };

    //some systems can process OnUpdate from UpdateViewsSystem
    //order of update is metter, because without it canvas views will be in invalid state during the frame

    const EditorSystemsManager* GetSystemsManager() const;
    EditorSystemsManager* GetSystemsManager();
    DAVA::TArc::ContextAccessor* accessor = nullptr;

private:
    //this class is designed to be used only by EditorSystemsManager
    friend class EditorSystemsManager;

    //Ask system for a new state
    //this state must be unique
    //if two different systems require different states at the same time - this is logical error
    //this method is not const because it can update cached states
    virtual EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput);
    //if system can not process input - OnInput method will not be called
    virtual bool CanProcessInput(DAVA::UIEvent* currentInput) const;
    //process input to realize state logic
    virtual void ProcessInput(DAVA::UIEvent* currentInput);
    //invalidate caches or prepare to work depending on states
    virtual void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState);
    virtual void OnDisplayStateChanged(EditorSystemsManager::eDisplayState currentState, EditorSystemsManager::eDisplayState previousState);
    virtual CanvasControls CreateCanvasControls();
    virtual void DeleteCanvasControls(const CanvasControls& canvasControls);

    //return order of update
    //-1 is reserverd, 0 wil be updated first
    virtual DAVA::int32 GetUpdateOrder() const;
    virtual void OnUpdate();
};
