#include "Modules/IssueNavigatorModule/IssueHelper.h"

#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Controls/ListView.h>
#include <TArc/Controls/QtBoxLayouts.h>

DAVA::int32 IssueHelper::NextIssueId()
{
    return nextIssueId++;
}
