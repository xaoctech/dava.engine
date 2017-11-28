#pragma once

#include "Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
class UIControl;
class Type;
}

class ControlNode;
class IssueNavigatorWidget;
class IssueHelper;

class EventsIssuesHandler : public PackageListener
{
public:
    EventsIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::int32 sectionId, IssueNavigatorWidget* widget, IssueHelper& issueHelper);
    ~EventsIssuesHandler() override = default;

    bool IsEventPropety(AbstractProperty* property);

    // PackageListener
    void ActivePackageNodeWasChanged(PackageNode* node) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlAddComponent(ControlNode* node, ComponentPropertiesSection* section) override;
    void ControlRemoveComponent(ControlNode* node, ComponentPropertiesSection* section) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

    struct EventIssue
    {
        ControlNode* node;
        const DAVA::Type* componentType;
        DAVA::FastName propertyName;
        DAVA::int32 issueId;
        bool toRemove = false;
    };

private:
    PackageNode* GetPackage() const;
    bool IsRootControl(const ControlNode* node) const;
    DAVA::String GetPathToControl(const ControlNode* node) const;

    void ValidateNode(ControlNode* node);
    void ValidateNodeForChildren(ControlsContainerNode* container);

    void CreateIssue(ControlNode* node, const DAVA::Type* componentType, const DAVA::String& propertyName);
    void UpdateNodeIssue(EventIssue& issue);

    void RemoveNodeIssues(ControlNode* node, bool recursive);
    void RemoveAllIssues();

    void SearchIssuesInPackage(PackageNode* package);

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    PackageListenerProxy packageListenerProxy;

    DAVA::int32 sectionId = 0;
    IssueNavigatorWidget* navigatorWidget = nullptr;

    IssueHelper& issueHelper;
    DAVA::Vector<EventIssue> issues;

    DAVA::UnorderedMap<const DAVA::Type*, DAVA::Set<DAVA::FastName>> componentsAndProperties;
};
