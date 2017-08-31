#include "EditorSystems/BaseEditorSystem.h"
#include "Modules/DocumentsModule/EditorData.h"

#include <TArc/Core/ContextAccessor.h>
#include <UI/UIEvent.h>

BaseEditorSystem::BaseEditorSystem(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

const EditorSystemsManager* BaseEditorSystem::GetSystemsManager() const
{
    using namespace DAVA::TArc;
    DataContext* globalContext = accessor->GetGlobalContext();
    EditorData* editorData = globalContext->GetData<EditorData>();
    DVASSERT(editorData != nullptr);
    const EditorSystemsManager* systemsManager = editorData->GetSystemsManager();
    DVASSERT(systemsManager != nullptr);
    return systemsManager;
}

EditorSystemsManager* BaseEditorSystem::GetSystemsManager()
{
    using namespace DAVA::TArc;
    DataContext* globalContext = accessor->GetGlobalContext();
    EditorData* editorData = globalContext->GetData<EditorData>();
    DVASSERT(editorData != nullptr);
    EditorSystemsManager* systemsManager = editorData->systemsManager.get();
    DVASSERT(systemsManager != nullptr);
    return systemsManager;
}

void BaseEditorSystem::ProcessInput(DAVA::UIEvent* /*currentInput*/)
{
}

bool BaseEditorSystem::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    return false;
}

EditorSystemsManager::eDragState BaseEditorSystem::RequireNewState(DAVA::UIEvent* currentInput)
{
    return EditorSystemsManager::NoDrag;
}

void BaseEditorSystem::OnDragStateChanged(EditorSystemsManager::eDragState /*currentState*/, EditorSystemsManager::eDragState /*previousState*/)
{
}

void BaseEditorSystem::OnDisplayStateChanged(EditorSystemsManager::eDisplayState /*currentState*/, EditorSystemsManager::eDisplayState /*previousState*/)
{
}

CanvasControls BaseEditorSystem::CreateCanvasControls() const
{
    return CanvasControls();
}

void BaseEditorSystem::DeleteCanvasControls(const CanvasControls& canvasControls)
{
}
