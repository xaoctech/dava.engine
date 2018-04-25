#include "TArc/DataProcessing/TArcAnyCasts.h"
#include "TArc/Utils/Utils.h"
#include "TArc/Qt/QtIcon.h"

#include <Base/Any.h>
#include <Base/FastName.h>
#include <Utils/StringFormat.h>

#include <Qt>
#include <QString>

namespace DAVA
{
Any StringToQString(const Any& value)
{
    return QString::fromStdString(value.Get<String>());
}

Any QStringToString(const Any& value)
{
    return value.Get<QString>().toStdString();
}

Any FastNameToQString(const Any& value)
{
    return QString(value.Get<FastName>().c_str());
}

Any QStringToFastName(const Any& value)
{
    return FastName(value.Get<QString>().toStdString());
}

Any CharPointerToQString(const Any& value)
{
    return QString(value.Get<const char*>());
}

template <typename T>
Any IntegralToQString(const Any& value)
{
    return QString::number(value.Get<T>());
}

// Don't create QString -> const char* cast function. It is impossible

Any BoolToCheckState(const Any& value)
{
    return value.Get<bool>() ? Qt::Checked : Qt::Unchecked;
}

Any CheckStateToBool(const Any& value)
{
    return value.Get<Qt::CheckState>() == Qt::Checked;
}

Any QColorToColorAny(const Any& value)
{
    return QColorToColor(value.Get<QColor>());
}

Any ColorToQColorAny(const Any& value)
{
    return ColorToQColor(value.Get<Color>());
}

Any ColorToQIcon(const Any& value)
{
    return QIcon(CreateIconFromColor(ColorToQColorAny(value).Get<QColor>()));
}

Any QColorToQIcon(const Any& value)
{
    return QIcon(CreateIconFromColor(value.Get<QColor>()));
}

Any ColorToString(const Any& value)
{
    const Color& c = value.Get<Color>();
    return Format("%f, %f, %f, %f", c.r, c.g, c.b, c.a);
}

void RegisterTArcAnyCasts()
{
    AnyCast<String, QString>::Register(&StringToQString);
    AnyCast<QString, String>::Register(&QStringToString);
    AnyCast<FastName, QString>::Register(&FastNameToQString);
    AnyCast<QString, FastName>::Register(&QStringToFastName);
    AnyCast<const char*, QString>::Register(&CharPointerToQString);
    AnyCast<int32, QString>::Register(&IntegralToQString<int32>);
    AnyCast<uint32, QString>::Register(&IntegralToQString<uint32>);
    AnyCast<int16, QString>::Register(&IntegralToQString<int16>);
    AnyCast<uint16, QString>::Register(&IntegralToQString<uint16>);
    AnyCast<int8, QString>::Register(&IntegralToQString<int8>);
    AnyCast<uint8, QString>::Register(&IntegralToQString<uint8>);
    AnyCast<int64, QString>::Register(&IntegralToQString<int64>);
    AnyCast<uint64, QString>::Register(&IntegralToQString<uint64>);
    AnyCast<size_t, QString>::Register(&IntegralToQString<size_t>);
    AnyCast<float32, QString>::Register(&IntegralToQString<float32>);
    AnyCast<float64, QString>::Register(&IntegralToQString<float64>);
    AnyCast<bool, Qt::CheckState>::Register(&BoolToCheckState);
    AnyCast<Qt::CheckState, bool>::Register(&CheckStateToBool);

    AnyCast<QColor, Color>::Register(&QColorToColorAny);
    AnyCast<Color, QColor>::Register(&ColorToQColorAny);
    AnyCast<Color, QIcon>::Register(&ColorToQIcon);
    AnyCast<QColor, QIcon>::Register(&QColorToQIcon);
    AnyCast<Color, String>::Register(&ColorToString);
}
} // namespace DAVA
