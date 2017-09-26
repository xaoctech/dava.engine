#pragma once

#include <TArc/DataProcessing/DataNode.h>

class EditorInput;

class InputModuleData : public DAVA::TArc::DataNode
{
public:
    InputModuleData();
    ~InputModuleData() override;

private:
    friend class InputModule;
    std::unique_ptr<EditorInput> system;

    DAVA_VIRTUAL_REFLECTION(InputModuleData, DAVA::TArc::DataNode);
};
