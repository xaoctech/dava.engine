#pragma once

#include <TArc/DataProcessing/DataNode.h>
#include <Base/RefPtr.h>
#include <UI/UIControl.h>

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

    DAVA_VIRTUAL_REFLECTION(CanvasModuleData, DAVA::TArc::DataNode);
};
