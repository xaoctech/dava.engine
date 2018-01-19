#pragma once

namespace DAVA
{
class DataContext;
}

class IssuesHandler
{
public:
    virtual ~IssuesHandler(){};
    virtual void OnContextActivated(DAVA::DataContext* current){};
    virtual void OnContextDeleted(DAVA::DataContext* current){};
};
