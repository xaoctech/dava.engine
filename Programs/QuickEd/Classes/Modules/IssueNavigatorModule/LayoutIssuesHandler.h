#pragma once

#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorModule.h"
#include "Classes/Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>
#include <UI/Layouts/UILayoutSystemListener.h>

#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class UI;
}

class UIControl;
class LayoutFormula;
}

class IssueNavigatorWidget;
class IndexGenerator;

class LayoutIssuesHandler : public IssuesHandler, DAVA::UILayoutSystemListener, PackageListener
{
public:
    LayoutIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, DAVA::int32 sectionId, IssueNavigatorWidget* widget, IndexGenerator& indexGenerator);
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

    IndexGenerator& indexGenerator;

    struct LayoutIssue
    {
        DAVA::int32 issueId = 0;
        ControlNode* controlNode = nullptr;
    };

    DAVA::Array<DAVA::UnorderedMap<DAVA::UIControl*, LayoutIssue>, DAVA::Vector2::AXIS_COUNT> createdIssues;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
    PackageListenerProxy packageListenerProxy;
};
