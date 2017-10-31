#pragma once

#include "Classes/Modules/PackageModule/Private/PackageWidget.h"

#include <TArc/DataProcessing/DataNode.h>

class PackageNode;

namespace DAVA
{
namespace TArc
{
class QtAction;
}
}

class PackageData : public DAVA::TArc::DataNode
{
public:
private:
    friend class PackageModule;

    DAVA::TArc::QtAction* copyAction = nullptr;
    DAVA::TArc::QtAction* pasteAction = nullptr;
    DAVA::TArc::QtAction* cutAction = nullptr;

    DAVA::TArc::QtAction* deleteAction = nullptr;
    DAVA::TArc::QtAction* duplicateAction = nullptr;

    DAVA::TArc::QtAction* jumpToPrototypeAction = nullptr;
    DAVA::TArc::QtAction* findPrototypeInstancesAction = nullptr;

    PackageWidget* packageWidget = nullptr;
    DAVA::Map<PackageNode*, PackageContext> packageWidgetContexts;

    DAVA_VIRTUAL_REFLECTION(PackageData, DAVA::TArc::DataNode);
};
