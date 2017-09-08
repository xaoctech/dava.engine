#pragma once

#include <TArc/DataProcessing/DataNode.h>

class LibraryWidget;

class LibraryData : public DAVA::TArc::DataNode
{
public:
    LibraryData() = default;

private:
    friend class LibraryModule;

    struct ActionInfo
    {
        DAVA::TArc::QtAction* action = nullptr;
        DAVA::TArc::ActionPlacementInfo placement;
    };
    using ActionsMap = DAVA::UnorderedMap<ControlNode*, ActionInfo>;

    LibraryWidget* libraryWidget = nullptr;

    ActionsMap controlsActions;
    ActionsMap prototypesActions;

    PackageNode* currentPackageNode = nullptr;

    DAVA_VIRTUAL_REFLECTION(LibraryData, DAVA::TArc::DataNode);
};