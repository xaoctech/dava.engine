#pragma once

#include "Classes/Modules/IssueNavigatorModule/IssueHandler.h"
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

class IndexGenerator;

class LayoutIssueHandler : public IssueHandler, DAVA::UILayoutSystemListener, PackageListener
{
public:
    LayoutIssueHandler(DAVA::ContextAccessor* accessor, DAVA::int32 sectionId, IndexGenerator* indexGenerator);
    ~LayoutIssueHandler() override;

private:
    // UILayoutSystemListener
    void OnFormulaProcessed(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;
    void OnFormulaRemoved(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula) override;

    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;

    void RemoveIssueForControl(DAVA::UIControl* control, DAVA::Vector2::eAxis axis);

    IndexGenerator* indexGenerator = nullptr;

    DAVA::Array<DAVA::UnorderedMap<DAVA::UIControl*, DAVA::int32>, DAVA::Vector2::AXIS_COUNT> createdIssues;

    PackageListenerProxy packageListenerProxy;
};
