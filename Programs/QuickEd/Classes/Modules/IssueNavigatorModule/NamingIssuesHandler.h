#pragma once

#include "Classes/Modules/IssueNavigatorModule/IssueNavigatorWidget.h"
#include "Classes/Modules/IssueNavigatorModule/IssueHandler.h"
#include "Classes/Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class UI;
}
class UIControl;
}

class ControlNode;
class IssueNavigatorWidget;
class IndexGenerator;

class NamingIssuesHandler : public IssuesHandler, PackageListener
{
public:
    NamingIssuesHandler(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui_, DAVA::int32 sectionId, IssueNavigatorWidget* widget, IndexGenerator& indexGenerator);
    ~NamingIssuesHandler() override = default;

    // IssuesHandler
    void OnContextDeleted(DAVA::TArc::DataContext* current) override;

    // PackageListener
    void ActivePackageNodeWasChanged(PackageNode* node) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

private:
    struct DuplicationsIssue
    {
        Issue issue;
        DAVA::UnorderedSet<ControlNode*> controls;
    };
    using DuplicationsIssuesMap = DAVA::UnorderedMap<DAVA::FastName, DuplicationsIssue>;

    struct PackageIssues
    {
        DAVA::TArc::DataContext* context = nullptr;
        DAVA::UnorderedMap<ControlNode*, Issue> symbolsIssues;
        DuplicationsIssuesMap duplicationIssues;
    };

    void ValidateNameSymbolsCorrectnessForChildren(ControlsContainerNode* node);
    void ValidateNameSymbolsCorrectness(ControlNode* node);
    void ValidateNameUniqueness(ControlNode* node);

    void CreateSymbolsIssue(ControlNode* node);
    void CreateDuplicationsIssue(ControlNode* node);
    void AddToDuplicationsIssue(DuplicationsIssue& issue, ControlNode* node);
    void UpdateSymbolsIssue(std::pair<ControlNode* const, Issue>& symbolsIssue);
    void UpdateDuplicationsIssue(DuplicationsIssue& issue);

    void RemoveSymbolsIssuesRecursively(ControlNode* node);
    void RemoveSymbolsIssue(ControlNode* node);
    void RemoveFromDuplicationsIssue(ControlNode* node);

    void RemoveIssuesFromPanel();
    void RestoreIssuesOnToPanel();

    void SearchIssuesInPackage(PackageNode* package);

    PackageNode* GetPackage() const;
    DAVA::UnorderedSet<ControlNode*> GetControlsByName(const DAVA::String& name);

    DuplicationsIssuesMap::iterator FindInDuplicationsIssues(ControlNode* node);

private:
    DAVA::int32 sectionId = 0;
    IssueNavigatorWidget* navigatorWidget = nullptr;

    IndexGenerator& indexGenerator;

    DAVA::UnorderedMap<PackageNode*, PackageIssues> packageIssues;
    PackageNode* currentPackage = nullptr;
    DAVA::UnorderedMap<ControlNode*, Issue>* symbolsIssues = nullptr;
    DuplicationsIssuesMap* duplicationIssues = nullptr;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::UI* ui = nullptr;
    PackageListenerProxy packageListenerProxy;
};
