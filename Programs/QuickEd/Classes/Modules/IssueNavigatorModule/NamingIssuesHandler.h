#pragma once

#include "Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
class UIControl;
}

class ControlNode;
class IssueNavigatorWidget;

class NamingIssuesHandler : public PackageListener
{
public:
    NamingIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::int32 sectionId, IssueNavigatorWidget* widget);
    ~NamingIssuesHandler() = default;

    // PackageListener
    void ActivePackageNodeWasChanged(PackageNode* node) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

private:
    void ValidateNameSymbolsCorrectnessRecursively(ControlsContainerNode* node);
    void ValidateNameSymbolsCorrectness(ControlNode* node);
    void ValidateNameUniqueness(ControlNode* node);

    void CreateSymbolsIssue(ControlNode* node);
    void CreateDuplicationsIssue(ControlNode* node);

    void RemoveSymbolsIssue(ControlNode* node);
    void RemoveDuplicationsIssue(ControlNode* node);
    void RemoveAllIssues();

    void SearchIssuesInPackage(PackageNode* package);

    PackageNode* GetPackage() const;
    bool IsRootControl(const ControlNode* node) const;
    DAVA::String GetPathToControl(const ControlNode* node) const;
    DAVA::UnorderedSet<ControlNode*> GetControlsByName(const DAVA::FastName& name);
    bool FindPreviousDuplicatedNameForControl(ControlNode* node, DAVA::FastName& previousName);


    DAVA::int32 sectionId = 0;
    IssueNavigatorWidget* widget = nullptr;

    DAVA::int32 nextIssueId = 0;

    DAVA::UnorderedMap<ControlNode*, DAVA::int32> symbolsIssues;
    DAVA::UnorderedMap<ControlNode*, DAVA::int32> duplicationIssues;

    DAVA::UnorderedMap<FastName, DAVA::UnorderedSet<ControlNode*>> duplicationControls;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    PackageListenerProxy packageListenerProxy;
};
