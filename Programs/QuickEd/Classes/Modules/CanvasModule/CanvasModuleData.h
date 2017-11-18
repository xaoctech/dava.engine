#pragma once

#include <Base/RefPtr.h>
#include <TArc/DataProcessing/DataNode.h>
#include <TArc/WindowSubSystem/UI.h>
#include <UI/UIControl.h>

#include <QString>

class EditorCanvas;
class EditorControlsView;

class CanvasModuleData : public DAVA::TArc::DataNode
{
public:
    CanvasModuleData();
    ~CanvasModuleData() override;

    const EditorCanvas* GetEditorCanvas() const;
    const EditorControlsView* GetEditorControlsView() const;

private:
    friend class CanvasModule;
    friend class EditorCanvas;
    friend class EditorControlsView;

    std::unique_ptr<EditorCanvas> editorCanvas;
    std::unique_ptr<EditorControlsView> controlsView;
    DAVA::RefPtr<DAVA::UIControl> canvas;

    struct ActionInfo
    {
        QString name;
        DAVA::TArc::ActionPlacementInfo placement;
    };
    DAVA::Vector<ActionInfo> bgrColorActions;

    DAVA_VIRTUAL_REFLECTION(CanvasModuleData, DAVA::TArc::DataNode);
};
