#pragma once

#include "TArc/DataProcessing/SettingsNode.h"
#include "TArc/Qt/QtByteArray.h"
#include "TArc/Qt/QtRect.h"

namespace DAVA
{
namespace TArc
{
class ColorPickerSettings : public DAVA::TArc::SettingsNode
{
public:
    ColorPickerSettings();

    float32 maxMultiplier = 2.0;
    QByteArray customPalette;
    QRect dialogGeometry;

    DAVA_VIRTUAL_REFLECTION(ColorPickerSettings, DAVA::TArc::SettingsNode);
};
} // namespace TArc
} // namespace DAVA