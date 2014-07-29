#include "PaletteHelper.h"

#include <QPainter>
#include <QLinearGradient>



QImage PaletteHelper::BuildHSVImage( QSize const& size )
{
    QImage img( size.width(), size.height(), QImage::Format_RGB32 );

    uint *p = reinterpret_cast<uint *>( img.scanLine( 0 ) );

    if ( size.width() == 1 || size.height() == 1 )
    {
        img.fill( Qt::black );
    }
    else
    {
        for ( int y = 0; y < size.height(); y++ )
        {
            const uint *end = p + size.width();
            int x = 0;
            while ( p < end )
            {
                const QPoint pt( x, y );
                QColor c;
                c.setHsv( hue( pt, size ), sat( pt, size ), val( pt, size ) );
                *p = c.rgb();
                p++;
                x++;
            }
        }
    }

    return img;
}

QImage PaletteHelper::BuildGradient( const QSize& size, const QColor& c1, const QColor& c2, bool hor, bool ver )
{
    QImage img( size.width(), size.height(), QImage::Format_RGB32 );

    const qreal x1 = 0;
    const qreal y1 = 0;
    const qreal x2 = hor ? size.width() : 0;
    const qreal y2 = ver ? size.height() : 0;

    QLinearGradient gradient( x1, y1, x2, y2 );
    gradient.setColorAt( 0.0, c1 );
    gradient.setColorAt( 1.0, c2 );

    QPainter p( &img );
    //p.fillRect( 0, 0, size.width(), size.height(), QBrush( Qt::white ) );
    p.fillRect( 0, 0, size.width(), size.height(), QBrush( gradient ) );

    return img;
}


int PaletteHelper::hue( QPoint const& pt, QSize const& size )
{
    return 360 - pt.x() * 360 / ( size.width() - 1 );
}

int PaletteHelper::sat( QPoint const& pt, QSize const& size )
{
    return 255 - pt.y() * 255 / ( size.height() - 1 );
}

int PaletteHelper::val( QPoint const& pt, QSize const& size )
{
    Q_UNUSED( pt );
    Q_UNUSED( size );

    return 240;
}
