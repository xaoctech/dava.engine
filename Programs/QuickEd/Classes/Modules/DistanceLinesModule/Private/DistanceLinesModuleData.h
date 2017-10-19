#pragma once

#include <TArc/DataProcessing/DataNode.h>

class DistanceSystem;

class DistanceLinesModuleData : public DAVA::TArc::DataNode
{
public:
    ~DistanceLinesModuleData() override;

private:
    friend class DistanceLinesModule;
    std::unique_ptr<DistanceSystem> system;

    DAVA_VIRTUAL_REFLECTION(DistanceLinesModuleData, DAVA::TArc::DataNode);
};
