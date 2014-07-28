#include "ColorPalette.h"

#include <QPainter>



ColorPalette::ColorPalette(QWidget *parent)
    : QFrame(parent)
{
}

ColorPalette::~ColorPalette()
{
}

void ColorPalette::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    if (cache.isNull())
    {
        const QRect rc(0, 0, width(), height());
        const int w = width();
        const int h = height();

        QImage img(width(), height(), QImage::Format_RGB32);
        const QSize size( rc.size() );
        uint *p = reinterpret_cast<uint *>( img.scanLine( 0 ) );

        if (size.width() == 1 || size.height() == 1)
        {
            img.fill(Qt::black);
        }
        else
        {
            for ( int y = 0; y < h; y++ )
            {
                const uint *end = p + w;
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

        cache = QPixmap::fromImage(img);
    }

    QPainter p(this);
    p.drawPixmap(0, 0, cache);
}

void ColorPalette::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED(e);
    cache = QPixmap();
}

int ColorPalette::hue( const QPoint& pt, const QSize& size )
{
    return 360 - pt.x() * 360 / ( size.width() - 1 );
}

int ColorPalette::sat( const QPoint& pt, const QSize& size )
{
    return 255 - pt.y() * 255 / ( size.height() - 1 );
}

int ColorPalette::val( const QPoint& pt, const QSize& size )
{
    Q_UNUSED(pt);
    Q_UNUSED(size);
    
    return 222;
}
