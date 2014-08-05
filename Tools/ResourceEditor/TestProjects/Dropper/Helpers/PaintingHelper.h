#ifndef PALETTEHELPER_H
#define PALETTEHELPER_H

#include <QObject>
#include <QColor>
#include <QImage>


class PaintingHelper
{
public:
    static QImage BuildHSVImage( const QSize& size );
    static QPoint GetHSVColorPoint( const QColor& c, const QSize& size );
    static QImage BuildGradient( const QSize& size, const QColor& c1, const QColor& c2, Qt::Orientation orientation );
    static QBrush BuildGridBrush( const QSize& size );
    static QImage BuildArrowIcon( const QSize& size, Qt::Edge dimension, const QColor& bgColor = Qt::lightGray, const QColor& brdColor = Qt::gray );

    static int HueRC( const QPoint& pt, const QSize& size );
    static int SatRC( const QPoint& pt, const QSize& size );
    static int ValRC( const QPoint& pt, const QSize& size );
};

#endif // PALETTEHELPER_H
