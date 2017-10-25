#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

#include <Base/BaseTypes.h>
#include <Math/Color.h>
#include <Reflection/Reflection.h>

class PreviewWidgetSettings : public DAVA::TArc::SettingsNode
{
public:
    static const DAVA::Color defaultBackgroundColor0;
    static const DAVA::Color defaultBackgroundColor1;
    static const DAVA::Color defaultBackgroundColor2;

    DAVA::Vector<DAVA::Color> backgroundColors = { defaultBackgroundColor0, defaultBackgroundColor1, defaultBackgroundColor2 };
    DAVA::uint32 backgroundColorIndex = 0;

    DAVA_VIRTUAL_REFLECTION(PreviewWidgetSettings, DAVA::TArc::SettingsNode);

private:
    // SettingsNode
    void Load(const DAVA::TArc::PropertiesItem& settingsNode) override;
    void Save(DAVA::TArc::PropertiesItem& settingsNode) const override;

    void LoadVersion0(const DAVA::TArc::PropertiesItem& settingsNode);
    void LoadVersion1(const DAVA::TArc::PropertiesItem& settingsNode, DAVA::Reflection& settingsReflection);
    void LoadDefaultValues();

    DAVA::uint32 version = 1;
};