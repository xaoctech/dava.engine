#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Reflection/Reflection.h>

class RenderOptionsData : public DAVA::TArc::DataNode
{
    DAVA_VIRTUAL_REFLECTION(RenderOptionsData, DAVA::TArc::DataNode);

public:
    bool IsEnabled() const;
};