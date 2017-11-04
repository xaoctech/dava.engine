#pragma once

#include "Classes/Interfaces/PackageActionsInterface.h"

#include <TArc/DataProcessing/DataListener.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class QMimeData;
class PackageBaseNode;

class PackageModule : public DAVA::TArc::ClientModule, public Interfaces::PackageActionsInterface, private DAVA::TArc::DataListener
{
    // ClientModule
    void PostInit() override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    // PackageActionsInterface
    QAction* GetCutAction() override;
    QAction* GetCopyAction() override;
    QAction* GetPasteAction() override;
    QAction* GetDuplicateAction() override;
    QAction* GetDeleteAction() override;
    QAction* GetJumpToPrototypeAction() override;
    QAction* GetFindPrototypeInstancesAction() override;

    // DataListener
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void InitData();
    void CreateActions();
    void CreatePackageWidget();
    void RegisterGlobalOperation();

    void OnImport();
    void OnAddStyle();
    void OnCut();
    void OnCopy();
    void OnPaste();
    void OnDuplicate();
    void OnCopyControlPath();
    void OnRename();
    void OnDelete();
    void OnMoveUp();
    void OnMoveDown();
    void OnMoveLeft();
    void OnMoveRight();
    void OnRunUIViewer();
    void OnRunUIViewerFast();
    void OnJumpToPrototype();
    void OnFindPrototypeInstances();
    void OnCollapseAll();

    void OnDropIntoPackageNode(const QMimeData* data, Qt::DropAction action, PackageBaseNode* destNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos);

    enum eDirection : bool
    {
        UP = true,
        DOWN = false
    };
    bool CanMove(const PackageBaseNode* dest, PackageBaseNode* node, eDirection direction) const;
    bool CanMoveLeft(PackageBaseNode* node) const;
    bool CanMoveRight(PackageBaseNode* node) const;
    void MoveNode(PackageBaseNode* node, eDirection direction);
    void MoveNode(PackageBaseNode* node, PackageBaseNode* dest, DAVA::uint32 destIndex);

    void PushErrorMessage(const DAVA::String& errorMessage);

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);

    DAVA::TArc::QtConnections connections;
    DAVA::TArc::DataWrapper documentDataWrapper;

    DAVA_VIRTUAL_REFLECTION(PackageModule, DAVA::TArc::ClientModule);
};
