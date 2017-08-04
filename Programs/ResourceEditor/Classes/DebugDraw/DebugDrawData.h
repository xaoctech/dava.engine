#pragma once

#include "Classes/DebugDraw/DebugDrawSystem.h"
#include "TArc/DataProcessing/DataNode.h"


class DebugDrawData : public DAVA::TArc::DataNode
{
public:

private:
    friend class DebugDrawModule;
    std::unique_ptr<DebugDrawSystem> debugDrawSystem;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DebugDrawData, DAVA::TArc::DataNode)
    {
    }
};

