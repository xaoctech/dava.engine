#pragma once

#include <Base/BaseTypes.h>
#include <UI/Layouts/UILayoutSystemListener.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}

class UIControl;
class LayoutFormula;
}

class IssueNavigatorWidget;
class IssueHelper;

class LayoutIssuesHandler : public DAVA::UILayoutSystemListener
{
public:
    LayoutIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::int32 sectionId, IssueNavigatorWidget* widget, IssueHelper& issueHelper);
    ~LayoutIssuesHandler() override;

    void OnFormulaProcessed(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;
    void OnFormulaRemoved(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;

private:
    void RemoveIssue(DAVA::UIControl* control, DAVA::Vector2::eAxis axis);
    bool IsRootControl(const DAVA::UIControl* control) const;

    DAVA::int32 sectionId = 0;
    IssueNavigatorWidget* widget = nullptr;

    IssueHelper& issueHelper;

    DAVA::Array<DAVA::UnorderedMap<DAVA::UIControl*, DAVA::int32>, DAVA::Vector2::AXIS_COUNT> createdIssues;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
};
