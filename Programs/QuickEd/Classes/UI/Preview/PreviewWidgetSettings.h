#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Color.h>
#include <Base/BaseTypes.h>

class PreviewWidgetSettings : public DAVA::TArc::SettingsNode
{
public:
    PreviewWidgetSettings();

    DAVA::Color backgroundColor0;
    DAVA::Color backgroundColor1;
    DAVA::Color backgroundColor2;
    DAVA::Vector<DAVA::Color> backgroundColors;
    DAVA::uint32 backgroundColorIndex = 0;

    DAVA_VIRTUAL_REFLECTION(PreviewWidgetSettings, DAVA::TArc::SettingsNode);

private:
    // SettingsNode
    void Load(const DAVA::TArc::PropertiesItem& settingsNode) override;
    void Save(DAVA::TArc::PropertiesItem& settingsNode) const override;
};