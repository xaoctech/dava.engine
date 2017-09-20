#pragma once

#include <TArc/DataProcessing/DataNode.h>

class PixelGrid;

class PixelGridData : public DAVA::TArc::DataNode
{
public:
    PixelGridData();
    ~PixelGridData() override;

private:
    friend class PixelGridModule;
    std::unique_ptr<PixelGrid> pixelGrid;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArc::DataNode);
};
