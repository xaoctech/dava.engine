#pragma once

#include "Modules/PackageModule/Private/PackageWidget.h"

#include <TArc/DataProcessing/DataNode.h>

class PackageNode;

class PackageData : public DAVA::TArc::DataNode
{
public:
private:
    friend class PackageModule;

    QAction* importPackageAction = nullptr;
    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;
    QAction* delAction = nullptr;
    QAction* duplicateControlsAction = nullptr;

    QAction* renameAction = nullptr;
    QAction* copyControlPathAction = nullptr;

    QAction* moveUpAction = nullptr;
    QAction* moveDownAction = nullptr;
    QAction* moveLeftAction = nullptr;
    QAction* moveRightAction = nullptr;

    QAction* runUIViewerFast = nullptr;
    QAction* runUIViewer = nullptr;

    PackageWidget* packageWidget = nullptr;
    DAVA::Map<PackageNode*, PackageContext> packageWidgetContexts;

    DAVA_VIRTUAL_REFLECTION(PackageData, DAVA::TArc::DataNode);
};
