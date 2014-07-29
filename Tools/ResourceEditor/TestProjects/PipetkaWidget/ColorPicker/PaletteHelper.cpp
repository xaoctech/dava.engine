#include "PaletteHelper.h"


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
