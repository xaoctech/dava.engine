#pragma once

#include <Base/BaseTypes.h>

class IssueHelper
{
    DAVA::int32 nextIssueId = 0;

public:
    IssueHelper(const IssueHelper&) = delete;
    IssueHelper() = default;
    ~IssueHelper() = default;

    DAVA::int32 NextIssueId();
};
