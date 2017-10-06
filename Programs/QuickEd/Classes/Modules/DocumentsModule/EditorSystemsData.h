#pragma once

#include <TArc/DataProcessing/DataNode.h>

class EditorSystemsManager;

class EditorSystemsData : public DAVA::TArc::DataNode
{
public:
    EditorSystemsData();
    ~EditorSystemsData() override;
    static DAVA::FastName emulationModePropertyName;

private:
    friend class BaseEditorSystem;
    friend class DocumentsModule;

    const EditorSystemsManager* GetSystemsManager() const;

    std::unique_ptr<EditorSystemsManager> systemsManager;
    bool emulationMode = false;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArc::DataNode);
};
