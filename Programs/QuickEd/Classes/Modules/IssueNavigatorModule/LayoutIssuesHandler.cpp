#include "Classes/Modules/IssueNavigatorModule/LayoutIssuesHandler.h"

#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Model/ControlProperties/AbstractProperty.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/IssueNavigatorModule/ControlNodeInfo.h"
#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorWidget.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Engine/Engine.h>
#include <UI/Layouts/LayoutFormula.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>

namespace LayoutIssuesHandlerDetails
{
using namespace DAVA;

ControlNode* FindControlNodeRecursively(ControlsContainerNode* container, UIControl* control)
{
    for (ControlNode* node : *container)
    {
        if (node->GetControl() == control)
        {
            return node;
        }
    }

    for (ControlNode* node : *container)
    {
        ControlNode* found = FindControlNodeRecursively(node, control);
        if (found != nullptr)
        {
            return found;
        }
    }

    return nullptr;
}

ControlNode* FindCorrespondingControlNode(PackageNode* packageNode, UIControl* control)
{
    ControlNode* found = FindControlNodeRecursively(packageNode->GetPackageControlsNode(), control);
    if (found == nullptr)
    {
        found = FindControlNodeRecursively(packageNode->GetPrototypes(), control);
    }

    return found;
}
}

LayoutIssuesHandler::LayoutIssuesHandler(DAVA::TArc::ContextAccessor* accessor_, DAVA::TArc::UI* ui_, DAVA::int32 sectionId_, IssueNavigatorWidget* widget_)
    : sectionId(sectionId_)
    , widget(widget_)
    , accessor(accessor_)
    , ui(ui_)
    , packageListenerProxy(this, accessor_)
{
    accessor->GetEngineContext()->uiControlSystem->GetLayoutSystem()->AddListener(this);
}

LayoutIssuesHandler::~LayoutIssuesHandler()
{
    accessor->GetEngineContext()->uiControlSystem->GetLayoutSystem()->RemoveListener(this);
}

void LayoutIssuesHandler::OnFormulaProcessed(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula)
{
    using namespace DAVA;

    if (formula->HasError())
    {
        auto it = createdIssues[axis].find(control);
        if (it != createdIssues[axis].end())
        {
            if (widget.isNull() == false)
            {
                widget->ChangeMessage(sectionId, it->second.issueId, formula->GetErrorMessage());
            }
        }
        else
        {
            const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
            DVASSERT(data != nullptr);

            ControlNode* controlNode = LayoutIssuesHandlerDetails::FindCorrespondingControlNode(data->GetPackageNode(), control);
            DVASSERT(controlNode != nullptr);

            Issue issue;
            issue.sectionId = sectionId;
            issue.issueId = nextIssueId;
            issue.message = formula->GetErrorMessage();
            issue.packagePath = data->GetPackagePath().GetFrameworkPath();
            issue.pathToControl = ControlNodeInfo::GetPathToControl(controlNode);
            issue.propertyName = axis == Vector2::AXIS_X ? "SizePolicy/horizontalFormula" : "SizePolicy/verticalFormula";

            nextIssueId++;
            if (widget.isNull() == false)
            {
                widget->AddIssue(issue);
            }

            LayoutIssue layoutIssue;
            layoutIssue.issueId = issue.issueId;
            layoutIssue.controlNode = controlNode;
            createdIssues[axis].emplace(control, std::move(layoutIssue));

            DAVA::TArc::NotificationParams notificationParams;
            notificationParams.title = "Error in formula";
            notificationParams.message = Result(Result::RESULT_ERROR, issue.message);
            ui->ShowNotification(DAVA::TArc::mainWindowKey, notificationParams);
        }
    }
    else
    {
        RemoveIssue(control, axis);
    }
}

void LayoutIssuesHandler::OnFormulaRemoved(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula)
{
    RemoveIssue(control, axis);
}

void LayoutIssuesHandler::RemoveIssue(DAVA::UIControl* control, DAVA::Vector2::eAxis axis)
{
    auto it = createdIssues[axis].find(control);
    if (it != createdIssues[axis].end())
    {
        if (widget.isNull() == false)
        {
            widget->RemoveIssue(sectionId, it->second.issueId);
        }
        createdIssues[axis].erase(it);
    }
}

void LayoutIssuesHandler::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    using namespace DAVA;

    if (property->GetName() == "Name")
    {
        const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(data != nullptr);

        UIControl* control = node->GetControl();

        for (auto& axisIssuesMap : createdIssues)
        {
            auto it = axisIssuesMap.find(control);
            if (it != axisIssuesMap.end())
            {
                LayoutIssue& issue = it->second;
                if (widget.isNull() == false)
                {
                    widget->ChangePathToControl(sectionId, issue.issueId, ControlNodeInfo::GetPathToControl(node));
                }
            }
        }
    }
}
