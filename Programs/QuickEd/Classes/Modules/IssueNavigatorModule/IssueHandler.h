#pragma once

#include <Base/BaseTypes.h>

#include "Modules/IssueNavigatorModule/IssueData.h"

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class DataContext;
}

class UIControl;
}

class IssueHandler
{
public:
    IssueHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::int32 sectionId);
    virtual ~IssueHandler();

    virtual void OnContextActivated(DAVA::TArc::DataContext* current){};
    virtual void OnContextDeleted(DAVA::TArc::DataContext* current){};

protected:
    IssueData* GetIssueData();
    bool IsControlOutOfScope(const DAVA::UIControl* control) const; // IsControlOutOfScope should be removed after DF-14277 implementing
    void AddIssue(DAVA::int32 id, const DAVA::String& message,
                  const DAVA::String& packagePath, const DAVA::String& controlPath,
                  const DAVA::String& propertyName);
    void AddIssue(const IssueData::Issue& issue);
    void ChangeMessage(DAVA::int32 id, const DAVA::String& message);
    void ChangePathToControl(DAVA::int32 id, const DAVA::String& pathToControl);
    void RemoveIssue(DAVA::int32 id);

    DAVA::int32 sectionId = 0;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
