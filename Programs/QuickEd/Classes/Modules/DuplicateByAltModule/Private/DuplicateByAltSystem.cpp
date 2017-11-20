#include "Classes/Modules/DuplicateByAltModule/Private/DuplicateByAltSystem.h"

#include <TArc/Utils/Utils.h>

#include <UI/UIEvent.h>

DuplicateByAltSystem::DuplicateByAltSystem(DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
{
}

eDragState DuplicateByAltSystem::RequireNewState(DAVA::UIEvent* currentInput, eInputSource inputSource)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    const EditorSystemsManager* systemsManager = GetSystemsManager();
    HUDAreaInfo areaInfo = GetSystemsManager()->GetCurrentHUDArea();
    if (duplicated == false
        && areaInfo.area != eArea::NO_AREA
        && currentInput->phase == UIEvent::Phase::DRAG
        && currentInput->mouseButton == eMouseButtons::LEFT
        && systemsManager->GetDragState() == eDragState::NoDrag
        && IsKeyPressed(eModifierKeys::ALT)
        && systemsManager->GetCurrentHUDArea().area == eArea::FRAME_AREA)
    {
        return eDragState::DuplicateByAlt;
    }
    return eDragState::NoDrag;
}

eSystems DuplicateByAltSystem::GetOrder() const
{
    return eSystems::DUPLICATE_BY_ALT;
}

void DuplicateByAltSystem::OnDragStateChanged(eDragState currentState, eDragState previousState)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (currentState == eDragState::DuplicateByAlt)
    {
        duplicateRequest.Emit();
        duplicated = true;
    }
}

bool DuplicateByAltSystem::CanProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) const
{
    return true;
}

void DuplicateByAltSystem::ProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource)
{
    using namespace DAVA;
    if (currentInput->device == eInputDevices::MOUSE &&
        currentInput->phase == UIEvent::Phase::ENDED &&
        currentInput->mouseButton == eMouseButtons::LEFT
        )
    {
        duplicated = false;
    }
}
