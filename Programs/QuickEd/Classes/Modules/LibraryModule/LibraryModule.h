#pragma once

#include <TArc/Core/ClientModule.h>

class LibraryWidget;
class ProjectData;

namespace DAVA
{
namespace TArc
{
class QtAction;
}
}

class LibraryModule : public DAVA::TArc::ClientModule, PackageListener
{
    struct ActionInfo
    {
        DAVA::TArc::QtAction* action = nullptr;
        DAVA::TArc::ActionPlacementInfo placement;
    };
    using ActionsMap = DAVA::UnorderedMap<ControlNode*, ActionInfo>;
    ActionsMap controlsActions;
    ActionsMap prototypesActions;

    void PostInit() override;
    void InitUI();
    void BindFields();

    void AddControlsMenus(const ProjectData* projectData, const Vector<RefPtr<PackageNode>>& libraryPackages);
    void RemoveControlsMenus();

    void AddPrototypesMenus(PackageNode* packageNode);
    void RemovePrototypesMenus();

    void AddImportedPackageControlsActions(const PackageNode* package);
    void RemoveImportedPackageControlsActions(const PackageNode* package);

    void ClearActions(ActionsMap&);

    void AddControlAction(ControlNode* controlNode, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, ActionsMap& actionsMap);
    void AddPackageControlsActions(PackageControlsNode* controls, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, ActionsMap& actionsMap);
    void RemoveControlAction(ControlNode* node, ActionsMap& actionsMap);

    void OnPackageChanged(const DAVA::Any& package);
    void OnProjectPathChanged(const DAVA::Any& projectPath);
    void OnControlCreateTriggered(ControlNode* node);

    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;

    DAVA::Vector<DAVA::RefPtr<PackageNode>> LoadLibraryPackages(ProjectData* projectData);

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::QtConnections connections;
    LibraryWidget* libraryWidget = nullptr;

    static QString controlsToolbarName;

    QMenu* otherControlsToolMenu = nullptr;
    QMenu* prototypesToolMenu = nullptr;
    QMenu* importedPrototypesToolMenu = nullptr;

    PackageNode* currentPackageNode = nullptr;

    DAVA_VIRTUAL_REFLECTION(LibraryModule, DAVA::TArc::ClientModule);
};
