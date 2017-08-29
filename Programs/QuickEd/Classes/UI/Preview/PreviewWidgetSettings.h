#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Color.h>
#include <Base/BaseTypes.h>

class PreviewWidgetSettings : public DAVA::TArc::SettingsNode
{
public:
    DAVA::Color backgroundColor0 = DAVA::Color::Transparent;
    DAVA::Color backgroundColor1 = DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f);
    DAVA::Color backgroundColor2 = DAVA::Color(0.0f, 0.0f, 0.0f, 1.0f);
    DAVA::uint32 backgroundColorIndex = 0;

    DAVA_VIRTUAL_REFLECTION(PreviewWidgetSettings, DAVA::TArc::SettingsNode);
};