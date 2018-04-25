#pragma once

#include "TArc/DataProcessing/SettingsNode.h"

#include <Reflection/Reflection.h>

#include <QRect>
#include <QByteArray>

namespace DAVA
{
class ColorPickerSettings : public SettingsNode
{
public:
    ColorPickerSettings();

    float32 maxMultiplier = 2.0;
    QByteArray customPalette;
    QRect dialogGeometry;

    DAVA_VIRTUAL_REFLECTION(ColorPickerSettings, SettingsNode);
};
} // namespace DAVA
