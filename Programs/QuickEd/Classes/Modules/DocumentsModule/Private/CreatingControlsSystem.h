#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"

#include <TArc/Core/FieldBinder.h>

#include <Math/Vector.h>

class CreatingControlsSystem final : public BaseEditorSystem
{
public:
    CreatingControlsSystem(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui);

private:
    // BaseEditorSystem
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;

    void OnAddingControlChanged(const DAVA::Any& control);
    void OnPackageChanged();

    bool IsDependsOnCurrentPackage(ControlNode* control);
    void AddControlAtPoint(const DAVA::Vector2& point);
    void ClearAddingTask();

private:
    DAVA::TArc::UI* ui = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    ControlNode* createFromControl = nullptr;
    bool controlDependsOnPackage = false;
};
