#pragma once

#include <TArc/DataProcessing/DataNode.h>

class EditorSystemsManager;
class ControlNode;

class EditorSystemsData : public DAVA::TArc::DataNode
{
public:
    EditorSystemsData();
    ~EditorSystemsData() override;

    ControlNode* GetHighlightedNode() const;

    static DAVA::FastName highlightedNodePropertyName;
    static DAVA::FastName emulationModePropertyName;

private:
    friend class BaseEditorSystem;
    friend class DocumentsModule;

    const EditorSystemsManager* GetSystemsManager() const;
    void SetHighlightedNode(ControlNode* node);

    ControlNode* highlightedNode = nullptr;
    std::unique_ptr<EditorSystemsManager> systemsManager;
    bool emulationMode = false;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArc::DataNode);
};
