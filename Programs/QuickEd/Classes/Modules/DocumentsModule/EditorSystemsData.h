#pragma once

#include <TArc/DataProcessing/DataNode.h>

class EditorSystemsManager;
class Painter;
class ControlNode;

class EditorSystemsData : public DAVA::TArc::DataNode
{
public:
    EditorSystemsData();
    ~EditorSystemsData() override;

    bool IsHighlightDisabled() const;

    static DAVA::FastName emulationModePropertyName;

private:
    friend class BaseEditorSystem;
    friend class DocumentsModule;

    const EditorSystemsManager* GetSystemsManager() const;

    std::unique_ptr<EditorSystemsManager> systemsManager;
    std::unique_ptr<Painter> painter;

    bool emulationMode = false;
    bool highlightDisabled = false;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArc::DataNode);
};
