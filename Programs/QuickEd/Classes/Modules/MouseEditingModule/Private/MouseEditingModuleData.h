#pragma once

#include <TArc/DataProcessing/DataNode.h>

class MouseEditingSystem;

class MouseEditingModuleData : public DAVA::TArc::DataNode
{
public:
    ~MouseEditingModuleData() override;
    std::unique_ptr<MouseEditingSystem> system;

    DAVA_VIRTUAL_REFLECTION(MouseEditingModuleData, DAVA::TArc::DataNode);
};
