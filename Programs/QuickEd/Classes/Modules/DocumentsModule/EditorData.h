#pragma once

#include <TArc/DataProcessing/DataNode.h>

class EditorSystemsManager;

class EditorData : public DAVA::TArc::DataNode
{
public:
    EditorData();
    ~EditorData() override;
    static DAVA::FastName emulationModePropertyName;
    static DAVA::FastName systemsManagerPropertyName;

    EditorSystemsManager* GetSystemsManager();

private:
    friend class BaseEditorSystem;
    friend class DocumentsModule;

    std::unique_ptr<EditorSystemsManager> systemsManager;
    bool emulationMode = false;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArc::DataNode);
};