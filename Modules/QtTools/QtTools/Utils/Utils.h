#pragma once

#include "Base/BaseTypes.h"
#include "Math/Color.h"
#include "FileSystem/FilePath.h"
#include "Debug/DVAssert.h"

#include <QString>
#include <QPixmap>
#include <QColor>
#include <QRegularExpression>

// Different string utilities.
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension);
bool FindAndReplace(DAVA::String& str, const DAVA::String& from, const DAVA::String& to);

QPixmap CreateIconFromColor(const QColor& color);

DAVA::Color QColorToColor(const QColor& qtColor);

QColor ColorToQColor(const DAVA::Color& davaColor);

void MakeAppForeground();
void RestoreMenuBar();

QString EscapeString(const QString& str);
QString UnescapeString(const QString& str);

DAVA::Vector<DAVA::float32> ParseFloatList(const DAVA::String& str);
DAVA::Vector<DAVA::float32> ParseFloatList(const QString& str);

template <typename T>
T StringToVector(const QString& str)
{
    static_assert(std::is_same<T, DAVA::Vector2>::value ||
                  std::is_same<T, DAVA::Vector3>::value ||
                  std::is_same<T, DAVA::Vector4>::value,
                  "this function works only for types Vector2, Vector3 and Vector4"
                  );

    DAVA::Vector<DAVA::float32> result = ParseFloatList(str);
    T vector;
    DAVA::int32 count = DAVA::Min(static_cast<DAVA::int32>(result.size()), static_cast<DAVA::int32>(T::AXIS_COUNT));
    for (DAVA::int32 i = 0; i < count; i++)
    {
        vector.data[i] = result[i];
    }
    return vector;
}
