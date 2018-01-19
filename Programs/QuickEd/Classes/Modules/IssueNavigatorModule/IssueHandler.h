#pragma once

namespace DAVA
{
namespace TArc
{
class DataContext;
}
}

class IssuesHandler
{
public:
    virtual ~IssuesHandler(){};
    virtual void OnContextActivated(DAVA::TArc::DataContext* current){};
    virtual void OnContextDeleted(DAVA::TArc::DataContext* current){};
};
