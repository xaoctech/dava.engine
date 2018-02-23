#include "VisualScriptEditor/Private/VisualScriptEditorDialogSettings.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptEditorDialogSettings)
{
    ReflectionRegistrator<VisualScriptEditorDialogSettings>::Begin()[M::DisplayName("Visual Script Editor Dialog"), M::SettingsSortKey(0)]
    .ConstructorByPointer()
    .Field("dialogGeometry", &VisualScriptEditorDialogSettings::dialogGeometry)[M::HiddenField()]
    .Field("recentScripts", &VisualScriptEditorDialogSettings::recentScripts)[M::HiddenField()]
    .End();
}
}
