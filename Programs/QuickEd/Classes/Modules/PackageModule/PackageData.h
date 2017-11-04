#pragma once

#include "Classes/Modules/PackageModule/Private/PackageWidget.h"

#include <TArc/DataProcessing/DataNode.h>

class PackageNode;
class QAction;

class PackageData : public DAVA::TArc::DataNode
{
public:
private:
    friend class PackageModule;

    QAction* copyAction = nullptr;
    QAction* pasteAction = nullptr;
    QAction* cutAction = nullptr;

    QAction* deleteAction = nullptr;
    QAction* duplicateAction = nullptr;

    QAction* jumpToPrototypeAction = nullptr;
    QAction* findPrototypeInstancesAction = nullptr;

    PackageWidget* packageWidget = nullptr;
    DAVA::Map<PackageNode*, PackageContext> packageWidgetContexts;

    DAVA_VIRTUAL_REFLECTION(PackageData, DAVA::TArc::DataNode);
};
