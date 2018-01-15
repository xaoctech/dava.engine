#pragma once

#include <TArc/DataProcessing/DataNode.h>

class IssueNavigatorData : public DAVA::TArc::DataNode
{
public:
    ~IssueNavigatorData() override;

private:
    friend class IssueNavigatorModule;

    DAVA_VIRTUAL_REFLECTION(IssueNavigatorData, DAVA::TArc::DataNode);
};
