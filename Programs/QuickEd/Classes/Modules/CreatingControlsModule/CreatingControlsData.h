#pragma once

#include <TArc/DataProcessing/DataNode.h>

class CreatingControlsSystem;

class CreatingControlsData : public DAVA::TArc::DataNode
{
public:
    CreatingControlsData() = default;

private:
    friend class CreatingControlsModule;
    CreatingControlsSystem* creatingControlsSystem = nullptr;

    DAVA_VIRTUAL_REFLECTION(CreatingControlsData, DAVA::TArc::DataNode);
};