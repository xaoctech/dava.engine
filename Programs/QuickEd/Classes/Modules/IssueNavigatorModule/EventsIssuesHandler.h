#pragma once

#include "Utils/PackageListenerProxy.h"
#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorModule.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Functional/Function.h>

namespace DAVA
{
class ContextAccessor;
class UI;
class UIControl;
class Type;
}

class ControlNode;
class IssueNavigatorWidget;
class IndexGenerator;
class IntrospectionProperty;

class EventsIssuesHandler : public IssuesHandler, PackageListener
{
public:
    EventsIssuesHandler(DAVA::ContextAccessor* accessor, DAVA::UI* ui, DAVA::int32 sectionId, IssueNavigatorWidget* widget, IndexGenerator& indexGenerator);
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
        bool wasFixed = false;
    };
    using MatchFunction = DAVA::Function<bool(const EventIssue&)>;

    DAVA::String CreateIncorrectSymbolsMessage(EventIssue& eventIssue);
    PackageNode* GetPackage() const;
    bool IsRootControl(const ControlNode* node) const;
    DAVA::String GetPathToControl(const ControlNode* node) const;

    void ValidateNode(ControlNode* node);
    void ValidateNodeForChildren(ControlsContainerNode* container);
    void ValidateSection(ControlNode* node, ComponentPropertiesSection* componentSection, bool removeFixedIssues);
    void ValidateProperty(ControlNode* node, const DAVA::Type* componentType, AbstractProperty* property, bool removeFixedIssues);

    void CreateIssue(ControlNode* node, const DAVA::Type* componentType, const DAVA::String& propertyName);
    void UpdateNodeIssue(EventIssue& issue);

    void RemoveIssuesIf(MatchFunction matchPred);
    void RemoveNodeIssues(ControlNode* node, bool recursive);
    void RemoveComponentIssues(ControlNode* node, const DAVA::Type* componentType);
    void RemoveAllIssues();

    void SearchIssuesInPackage(PackageNode* package);

    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::UI* ui = nullptr;
    PackageListenerProxy packageListenerProxy;

    DAVA::int32 sectionId = 0;
    IssueNavigatorWidget* navigatorWidget = nullptr;

    IndexGenerator& indexGenerator;
    DAVA::Vector<EventIssue> issues;

    DAVA::UnorderedMap<const DAVA::Type*, DAVA::Set<DAVA::FastName>> componentsAndProperties;
};
