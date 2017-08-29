#include "UI/Preview/PreviewWidgetSettings.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PreviewWidgetSettings)
{
    DAVA::ReflectionRegistrator<PreviewWidgetSettings>::Begin()[DAVA::M::DisplayName("Preview widget"), DAVA::M::SettingsSortKey(70)]
    .ConstructorByPointer()
    .Field("backgroundColor0", &PreviewWidgetSettings::backgroundColor0)[DAVA::M::DisplayName("Background color 0")]
    .Field("backgroundColor1", &PreviewWidgetSettings::backgroundColor1)[DAVA::M::DisplayName("Background color 1")]
    .Field("backgroundColor2", &PreviewWidgetSettings::backgroundColor2)[DAVA::M::DisplayName("Background color 2")]
    .Field("backgroundColorIndex", &PreviewWidgetSettings::backgroundColorIndex)[DAVA::M::DisplayName("Background color index"), DAVA::M::HiddenField()]
    .End();
}
