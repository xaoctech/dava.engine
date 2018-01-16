#pragma once

#include "Classes/Modules/IssueNavigatorModule/IssueHelper.h"
#include "Classes/Modules/IssueNavigatorModule/IssueHandler.h"

#include <TArc/DataProcessing/DataNode.h>

class IssueNavigatorData : public DAVA::TArc::DataNode
{
public:
    ~IssueNavigatorData() override;

private:
    friend class IssueNavigatorModule;

    DAVA::Vector<std::unique_ptr<IssueHandler>> handlers;
    IndexGenerator indexGenerator;

    DAVA_VIRTUAL_REFLECTION(IssueNavigatorData, DAVA::TArc::DataNode);
};
