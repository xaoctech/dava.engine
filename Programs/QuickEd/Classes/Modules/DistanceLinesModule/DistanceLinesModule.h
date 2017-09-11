#pragma once

#include <TArc/Core/ClientModule.h>

class DistanceModule : public DAVA::TArc::ClientModule
{
    DistanceModule();

    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(DistanceModule, DAVA::TArc::ClientModule);
};

class DistanceSystemPreferences : public DAVA::TArc::SettingsNode
{
public:
    DAVA::Color linesColor = DAVA::Color(1.0f, 0.0f, 0.0f, 0.9f);
    DAVA::Color textColor = DAVA::Color(1.0f, 0.0f, 0.0f, 0.9f);
    DAVA::String helpLinesTexture = DAVA::String("~res:/QuickEd/UI/HUDControls/MagnetLine/dotline.png");

    DAVA_VIRTUAL_REFLECTION(DistanceSystemPreferences, DAVA::TArc::SettingsNode);
};
