#pragma once

#include "EditorSystems/SelectionContainer.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class ControlNode;

class GroupingControlsModule : public DAVA::TArc::ClientModule
{
public:
    GroupingControlsModule();

private:
    // ClientModule
    void PostInit() override;

    void DoGroup();
    void DoUngroup();

    DAVA::Result CanGroupSelectedNodes(const SelectedNodes& selectedNodes) const;
    DAVA::Result CanUngroupNode(ControlNode* node) const;

    DAVA::TArc::QtConnections connections;
    DAVA::TArc::DataWrapper documentDataWrapper;
    DAVA::RefPtr<ControlNode> sampleGroupNode;

    DAVA_VIRTUAL_REFLECTION(GroupingControlsModule, DAVA::TArc::ClientModule);
};
