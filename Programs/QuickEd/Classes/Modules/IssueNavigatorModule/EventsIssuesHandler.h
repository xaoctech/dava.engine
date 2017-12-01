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
class IndexGenerator;

class EventsIssuesHandler : public PackageListener
{
public:
    EventsIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::int32 sectionId, IssueNavigatorWidget* widget, IndexGenerator& indexGenerator);
    ~EventsIssuesHandler() override = default;


    // PackageListener
    void ActivePackageNodeWasChanged(PackageNode* node) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlComponentWasAdded(ControlNode* node, ComponentPropertiesSection* section) override;
    void ControlComponentWasRemoved(ControlNode* node, ComponentPropertiesSection* section) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

private:
    struct EventIssue
    {
        ControlNode* node = nullptr;
        const DAVA::Type* componentType = nullptr;
        DAVA::FastName propertyName;
        DAVA::int32 issueId = 0;
        bool toRemove = false;
    };

    DAVA::String CreateIncorrectSymbolsMessage(EventIssue& eventIssue);
    bool IsEventPropety(AbstractProperty* property);
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

    IndexGenerator& indexGenerator;
    DAVA::Vector<EventIssue> issues;

    DAVA::UnorderedMap<const DAVA::Type*, DAVA::Set<DAVA::FastName>> componentsAndProperties;
};
