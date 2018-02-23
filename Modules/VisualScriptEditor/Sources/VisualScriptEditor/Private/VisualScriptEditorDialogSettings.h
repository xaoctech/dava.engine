#pragma once

#include "TArc/DataProcessing/SettingsNode.h"
#include "TArc/Qt/QtRect.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class VisualScriptEditorDialogSettings : public SettingsNode
{
public:
    QRect dialogGeometry;
    Vector<String> recentScripts;
    DAVA_VIRTUAL_REFLECTION(VisualScriptEditorDialogSettings, SettingsNode);
};

} //DAVA
