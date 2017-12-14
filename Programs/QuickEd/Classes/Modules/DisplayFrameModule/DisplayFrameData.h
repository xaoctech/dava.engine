#pragma once

#include <TArc/DataProcessing/DataNode.h>

class DisplaySafeArea;

class DisplayFrameData : public DAVA::TArc::DataNode
{
public:
    ~DisplayFrameData() override;

private:
    friend class DisplayFrameModule;
    std::unique_ptr<DisplaySafeArea> safeArea;

    DAVA_VIRTUAL_REFLECTION(DisplayFrameData, DAVA::TArc::DataNode);
};
