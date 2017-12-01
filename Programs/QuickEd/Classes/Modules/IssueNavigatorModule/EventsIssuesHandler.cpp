#include "Classes/Modules/IssueNavigatorModule/EventsIssuesHandler.h"

#include "Classes/Model/ControlProperties/AbstractProperty.h"
#include "Classes/Model/ControlProperties/NameProperty.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/IssueNavigatorModule/IssueHelper.h"
#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorWidget.h"
#include <Model/ControlProperties/ComponentPropertiesSection.h>

#include <Base/Type.h>
#include <UI/Events/UIInputEventComponent.h>
#include <UI/Events/UIMovieEventComponent.h>
#include <UI/UIControl.h>
#include <UI/UIControlHelpers.h>
#include <UI/UIControlSystem.h>

#include <TArc/Core/ContextAccessor.h>

EventsIssuesHandler::EventsIssuesHandler(DAVA::TArc::ContextAccessor* accessor_, DAVA::int32 sectionId_, IssueNavigatorWidget* widget_, IndexGenerator& indexGenerator_)
    : sectionId(sectionId_)
    , navigatorWidget(widget_)
    , accessor(accessor_)
    , packageListenerProxy(this, accessor_)
    , indexGenerator(indexGenerator_)
{
    using namespace DAVA;
    componentsAndProperties[Type::Instance<UIInputEventComponent>()] = {
        FastName("onTouchDown"),
        FastName("onTouchUpInside"),
        FastName("onTouchUpOutside"),
        FastName("onValueChanged"),
        FastName("onHoverSet"),
        FastName("onHoverRemoved")
    };
    componentsAndProperties[Type::Instance<UIMovieEventComponent>()] = {
        FastName("onStart"),
        FastName("onStop")
    };
}

DAVA::String EventsIssuesHandler::CreateIncorrectSymbolsMessage(EventIssue& eventIssue)
{
    return DAVA::Format("Node '%s' event property '%s' contains incorrect symbols", eventIssue.node->GetName().c_str(), eventIssue.propertyName.c_str());
}

bool EventsIssuesHandler::IsEventPropety(AbstractProperty* property)
{
    ComponentPropertiesSection* componentSection = dynamic_cast<ComponentPropertiesSection*>(property->GetParent());
    if (componentSection)
    {
        auto it = componentsAndProperties.find(componentSection->GetComponentType());
        if (it != componentsAndProperties.end())
        {
            return it->second.find(DAVA::FastName(property->GetName())) != it->second.end();
        }
    }
    return false;
}

PackageNode* EventsIssuesHandler::GetPackage() const
{
    return accessor->GetActiveContext()->GetData<DocumentData>()->GetPackageNode();
}

bool EventsIssuesHandler::IsRootControl(const ControlNode* node) const
{
    PackageNode* package = GetPackage();
    return ((node->GetParent() == package->GetPackageControlsNode()) || (node->GetParent() == package->GetPrototypes()));
}

DAVA::String EventsIssuesHandler::GetPathToControl(const ControlNode* node) const
{
    auto GetParentNode = [&](const ControlNode* node) -> ControlNode*
    {
        return IsRootControl(node) ? nullptr : dynamic_cast<ControlNode*>(node->GetParent());
    };

    DAVA::String pathToControl = node->GetName();

    for (const ControlNode* nextNode = GetParentNode(node);
         nextNode != nullptr;
         nextNode = GetParentNode(nextNode))
    {
        pathToControl = nextNode->GetName() + "/" + pathToControl;
    }

    return pathToControl;
}

void EventsIssuesHandler::CreateIssue(ControlNode* node, const DAVA::Type* componentType, const DAVA::String& propertyName)
{
    DAVA::int32 issueId = indexGenerator.NextIssueId();

    EventIssue eventIssue;
    eventIssue.node = node;
    eventIssue.componentType = componentType;
    eventIssue.propertyName = DAVA::FastName(propertyName.c_str());
    eventIssue.issueId = issueId;
    eventIssue.toRemove = false;
    issues.push_back(eventIssue);

    Issue issue;
    issue.sectionId = sectionId;
    issue.issueId = issueId;
    issue.message = CreateIncorrectSymbolsMessage(eventIssue);
    issue.packagePath = GetPackage()->GetPath().GetFrameworkPath();
    issue.pathToControl = GetPathToControl(node);
    issue.propertyName = propertyName;

    node->AddIssue(issue.issueId);
    navigatorWidget->AddIssue(issue);
}

void EventsIssuesHandler::UpdateNodeIssue(EventIssue& eventIssue)
{
    ControlNode* node = eventIssue.node;
    DAVA::int32 issueId = eventIssue.issueId;
    navigatorWidget->ChangeMessage(sectionId, issueId, CreateIncorrectSymbolsMessage(eventIssue));
    navigatorWidget->ChangePathToControl(sectionId, issueId, GetPathToControl(node));
}

void EventsIssuesHandler::RemoveNodeIssues(ControlNode* node, bool recursive)
{
    auto RemoveIssue = [&](EventIssue& issue)
    {
        if (issue.node == node)
        {
            DAVA::int32 issueId = issue.issueId;
            navigatorWidget->RemoveIssue(sectionId, issueId);
            node->RemoveIssue(issueId);
            return true;
        }
        return false;
    };

    auto it = std::remove_if(issues.begin(), issues.end(), RemoveIssue);
    issues.erase(it, issues.end());

    if (recursive)
    {
        for (ControlNode* child : *node)
        {
            RemoveNodeIssues(child, recursive);
        }
    }
}

void EventsIssuesHandler::RemoveAllIssues()
{
    auto RemoveIssue = [&](EventIssue& issue)
    {
        ControlNode* node = issue.node;
        const DAVA::int32& issueId = issue.issueId;
        node->RemoveIssue(issueId);
        navigatorWidget->RemoveIssue(sectionId, issueId);
    };

    std::for_each(issues.begin(), issues.end(), RemoveIssue);
    issues.clear();
}

void EventsIssuesHandler::SearchIssuesInPackage(PackageNode* package)
{
    if (package)
    {
        PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
        PackageControlsNode* packagePrototypesNode = package->GetPrototypes();

        ValidateNodeForChildren(packageControlsNode);
        ValidateNodeForChildren(packagePrototypesNode);

        package->AddListener(this);
    }
}

//////////////////////////////////////////////////////////////////////////

void EventsIssuesHandler::ActivePackageNodeWasChanged(PackageNode* package)
{
    RemoveAllIssues();
    SearchIssuesInPackage(package);
}

void EventsIssuesHandler::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (IsEventPropety(property))
    {
        ValidateNode(node);
    }

    if (property->GetName() == "Name")
    {
        // Refresh all children issues
        for (EventIssue& issue : issues)
        {
            if (node->IsParentOf(issue.node))
            {
                UpdateNodeIssue(issue);
            }
        }
    }
}

void EventsIssuesHandler::ControlComponentWasAdded(ControlNode* node, ComponentPropertiesSection* section)
{
    DVASSERT(section != nullptr);
    if (componentsAndProperties.find(section->GetComponentType()) != componentsAndProperties.end())
    {
        ValidateNode(node);
    }
}

void EventsIssuesHandler::ControlComponentWasRemoved(ControlNode* node, ComponentPropertiesSection* section)
{
    DVASSERT(section != nullptr);
    if (componentsAndProperties.find(section->GetComponentType()) != componentsAndProperties.end())
    {
        ValidateNode(node);
    }
}

void EventsIssuesHandler::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    ValidateNode(node);
    ValidateNodeForChildren(node);
}

void EventsIssuesHandler::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    RemoveNodeIssues(node, true);
}

//////////////////////////////////////////////////////////////////////////

void EventsIssuesHandler::ValidateNodeForChildren(ControlsContainerNode* container)
{
    for (ControlNode* node : *container)
    {
        ValidateNode(node);
        ValidateNodeForChildren(node);
    }
}

void EventsIssuesHandler::ValidateNode(ControlNode* node)
{
    using namespace DAVA;

    DAVA::int32 issuesToRemoveCount = 0;
    for (EventIssue& issue : issues)
    {
        issue.toRemove = (issue.node == node);
        issuesToRemoveCount += (issue.toRemove ? 1 : 0);
    }

    for (ComponentPropertiesSection* componentSection : node->GetRootProperty()->GetComponents())
    {
        // Is event component
        const Type* componentType = componentSection->GetComponentType();
        auto it = componentsAndProperties.find(componentType);
        if (it != componentsAndProperties.end())
        {
            Set<FastName>& componentProperties = it->second;
            // For each component from section properties
            for (auto& property : *componentSection)
            {
                // Is event property?
                FastName propertyName(property->GetName());
                if (componentProperties.find(propertyName) != componentProperties.end())
                {
                    Any propertyValue = property->GetValue();
                    FastName event = propertyValue.CanCast<FastName>() ? propertyValue.Cast<FastName>() : FastName();
                    // Check event format
                    if (UIControlHelpers::IsEventNameValid(event) == false)
                    {
                        auto IssueMatcher = [&](const EventIssue& issue)
                        {
                            return issue.node == node && issue.componentType == componentType && issue.propertyName == propertyName;
                        };
                        auto issuesIt = std::find_if(issues.begin(), issues.end(), IssueMatcher);

                        if (issuesIt == issues.end())
                        {
                            CreateIssue(node, componentType, property->GetName());
                        }
                        else
                        {
                            UpdateNodeIssue(*issuesIt);
                            issuesIt->toRemove = false;
                            issuesToRemoveCount--;
                        }
                    }
                }
            }
        }
    }
    // Remove fixed issues
    if (issuesToRemoveCount > 0)
    {
        auto it = std::remove_if(issues.begin(), issues.end(), [&](const EventIssue& issue) {
            if (issue.toRemove)
            {
                ControlNode* node = issue.node;
                const DAVA::int32& issueId = issue.issueId;
                node->RemoveIssue(issueId);
                navigatorWidget->RemoveIssue(sectionId, issueId);
                return true;
            }
            return false;
        });
        issues.erase(it, issues.end());
    }
}
