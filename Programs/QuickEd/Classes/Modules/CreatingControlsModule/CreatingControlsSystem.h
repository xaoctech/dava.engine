#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"

#include <TArc/Core/FieldBinder.h>

#include <Math/Vector.h>

class CreatingControlsSystem final : public BaseEditorSystem
{
public:
    CreatingControlsSystem(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui);

    // BaseEditorSystem
    eSystems GetOrder() const override;

    void SetCreateByClick(const DAVA::String& controlYamlString);
    void CancelCreateByClick();

private:
    // BaseEditorSystem
    EditorSystemsManager::eDragState RequireNewState(DAVA::UIEvent* currentInput) override;
    bool CanProcessInput(DAVA::UIEvent* currentInput) const override;
    void ProcessInput(DAVA::UIEvent* currentInput) override;
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;

    void BindFields();

    void OnPackageChanged(const DAVA::Any& package);
    void OnProjectPathChanged(const DAVA::Any& projectPath);

    void AddControlAtPoint(const DAVA::Vector2& point);

private:
    DAVA::TArc::UI* ui = nullptr;
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::DataWrapper documentDataWrapper;
    DAVA::String controlYamlString;
};
