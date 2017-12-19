#pragma once

#include "Classes/Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>
#include <UI/Layouts/UILayoutSystemListener.h>

#include <QPointer>

namespace DAVA
{
class ContextAccessor;
class UI;
class UIControl;
class LayoutFormula;
}

class IssueNavigatorWidget;

class LayoutIssuesHandler : public IssuesHandler, DAVA::UILayoutSystemListener, PackageListener
{
public:
    LayoutIssuesHandler(DAVA::ContextAccessor* accessor, DAVA::UI* ui, DAVA::int32 sectionId, IssueNavigatorWidget* widget);
    ~LayoutIssuesHandler() override;

private:
    // UILayoutSystemListener
    void OnFormulaProcessed(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;
    void OnFormulaRemoved(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;

    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;

    void RemoveIssue(DAVA::UIControl* control, DAVA::Vector2::eAxis axis);

    DAVA::int32 sectionId = 0;
    QPointer<IssueNavigatorWidget> widget = nullptr;

    DAVA::int32 nextIssueId = 0;

    struct LayoutIssue
    {
        DAVA::int32 issueId = 0;
        ControlNode* controlNode = nullptr;
    };

    DAVA::Array<DAVA::UnorderedMap<DAVA::UIControl*, LayoutIssue>, DAVA::Vector2::AXIS_COUNT> createdIssues;

    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::UI* ui = nullptr;
    PackageListenerProxy packageListenerProxy;
};
