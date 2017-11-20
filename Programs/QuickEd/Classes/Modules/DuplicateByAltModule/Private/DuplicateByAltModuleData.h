#pragma once

#include <TArc/DataProcessing/DataNode.h>

class DuplicateByAltSystem;

class DuplicateByAltModuleData : public DAVA::TArc::DataNode
{
public:
    ~DuplicateByAltModuleData() override;
    std::unique_ptr<DuplicateByAltSystem> system;

    DAVA_VIRTUAL_REFLECTION(DuplicateByAltModuleData, DAVA::TArc::DataNode);
};
