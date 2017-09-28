#pragma once

#include <TArc/Core/ClientModule.h>
#include "Modules/LibraryModule/LibraryData.h"

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
    void PostInit() override;

    void InitData();
    void InitUI();
    void BindFields();

    void AddProjectControls(const ProjectData* projectData, const Vector<RefPtr<PackageNode>>& libraryPackages);
    void RemoveProjectControls();
    void SetProjectControlsEnabled(bool active);

    void AddProjectPinnedControls(const ProjectData* projectData, const Vector<RefPtr<PackageNode>>& libraryPackages);
    void AddProjectLibraryControls(const ProjectData* projectData, const Vector<RefPtr<PackageNode>>& libraryPackages);
    void AddDefaultControls();

    void AddPrototypes(const PackageNode* packageNode, const QUrl& menuPoint, const QUrl& toolbarMenuPoint);

    void AddPackagePrototypes(PackageNode* packageNode);
    void RemovePackagePrototypes();

    void AddImportedPrototypes(const PackageNode* importedPackage);
    void RemoveImportedPrototypes(const PackageNode* importedPackage);

    void ClearActions(LibraryData::ActionsMap&);

    void AddControlAction(ControlNode* controlNode, bool isPrototype, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, LibraryData::ActionsMap& actionsMap);
    void RemoveControlAction(ControlNode* node, LibraryData::ActionsMap& actionsMap);

    void OnPackageChanged(const DAVA::Any& package);
    void OnProjectPathChanged(const DAVA::Any& projectPath);
    void OnControlCreateTriggered(ControlNode* node, bool makePrototype);

    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;

    DAVA::Vector<DAVA::RefPtr<PackageNode>> LoadLibraryPackages(ProjectData* projectData);

    LibraryData* GetLibraryData();

    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(LibraryModule, DAVA::TArc::ClientModule);
};
