#pragma once

#include "Application/QEGlobal.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "Utils/PackageListenerProxy.h"

#include <TArc/Core/ControllerModule.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Utils/QtDelayedExecutor.h>

class FindInDocumentController;
class PreviewWidget;
class EditorSystemsManager;
class PackageNode;
class ControlNode;

class DocumentsModule : public DAVA::TArc::ControllerModule, PackageListener
{
public:
    DocumentsModule();
    ~DocumentsModule() override;

protected:
    void OnRenderSystemInitialized(DAVA::Window* window) override;
    bool CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText) override;
    bool SaveOnWindowClose(const DAVA::TArc::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::TArc::WindowKey& key) override;

    void PostInit() override;
    void OnWindowClosed(const DAVA::TArc::WindowKey& key) override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

private:
    void InitCentralWidget();
    void InitGlobalData();

    void CreateDocumentsActions();
    void RegisterOperations();

    //Edit
    void CreateEditActions();
    void OnUndo();
    void OnRedo();

    //View
    void CreateViewActions();
    void CreateFindActions();

    void OpenPackageFiles(const QStringList& links);
    DAVA::TArc::DataContext::ContextID OpenDocument(const QString& path);
    DAVA::RefPtr<PackageNode> CreatePackage(const QString& path);

    void CloseDocument(DAVA::uint64 id);
    void CloseAllDocuments();
    void DeleteAllDocuments();
    void CloseDocuments(const DAVA::Set<DAVA::TArc::DataContext::ContextID>& ids);

    void ReloadCurrentDocument();
    void ReloadDocument(const DAVA::TArc::DataContext::ContextID& contextID);
    void ReloadDocuments(const DAVA::Set<DAVA::TArc::DataContext::ContextID>& ids);

    bool HasUnsavedDocuments() const;
    bool SaveDocument(const DAVA::TArc::DataContext::ContextID& contextID);
    bool SaveAllDocuments();
    bool SaveCurrentDocument();
    void DiscardUnsavedChanges();

    void SelectControl(const QString& documentPath, const QString& controlPath);

    void OnEmulationModeChanged(bool mode);

    //previewWidget helper functions
    void ChangeControlText(ControlNode* node);

    //documents watcher
    void OnFileChanged(const QString& path);
    void OnApplicationStateChanged(Qt::ApplicationState state);

    void ApplyFileChanges();
    DAVA::TArc::DataContext::ContextID GetContextByPath(const QString& path) const;

    void OnDragStateChanged(EditorSystemsManager::eDragState dragState, EditorSystemsManager::eDragState previousState);
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;

    void OnSelectInFileSystem();
    void OnDroppingFile(bool droppingFile);

    QPointer<PreviewWidget> previewWidget = nullptr;
    DAVA::TArc::QtConnections connections;

    DAVA::TArc::QtDelayedExecutor delayedExecutor;

    PackageListenerProxy packageListenerProxy;

    DAVA_VIRTUAL_REFLECTION(DocumentsModule, DAVA::TArc::ControllerModule);
};
