#include "QtDavaConvertion.h"
#include <QString>
#include <QColor>
#include <QVector2D>


DAVA::String QStringToString(const QString &str)
{
    return DAVA::String(str.toStdString());
}

QString StringToQString(const DAVA::String &str)
{
    return QString::fromStdString(str);
}

DAVA::WideString QStringToWideString(const QString& str)
{
#ifdef __DAVAENGINE_MACOS__
    return str.toStdWString();
#else
    return DAVA::WideString((wchar_t*)str.unicode(), str.length());
#endif
}

QString WideStringToQString(const DAVA::WideString& str)
{
#ifdef __DAVAENGINE_MACOS__
    return QString::fromStdWString(str);
#else
    return QString((const QChar*)str.c_str(), str.length());
#endif
}

DAVA::Color QColorToColor( const QColor& qtColor )
{
    return DAVA::Color(qtColor.redF(), qtColor.greenF(), qtColor.blueF(), qtColor.alphaF());
}

QColor ColorToQColor( const DAVA::Color& davaColor )
{
    return QColor(davaColor.r * 0xFF, davaColor.g * 0xFF, davaColor.b * 0xFF, davaColor.a * 0xFF);
}

DAVA::Vector2 QVector2DToVector2( const QVector2D &vector )
{
    return DAVA::Vector2(vector.x(), vector.y());
}

QVector2D Vector2ToQVector2D( const DAVA::Vector2 &vector )
{
    return QVector2D(vector.x, vector.y);
}
