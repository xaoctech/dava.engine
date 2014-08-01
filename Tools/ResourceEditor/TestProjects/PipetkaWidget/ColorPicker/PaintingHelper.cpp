#include "PaintingHelper.h"

#include <QPainter>
#include <QLinearGradient>



QImage PaintingHelper::BuildHSVImage( QSize const& size )
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

QImage PaintingHelper::BuildGradient( const QSize& size, const QColor& c1, const QColor& c2, bool hor, bool ver )
{
    QImage img( size, QImage::Format_ARGB32 );
    img.fill( Qt::transparent );

    const qreal x1 = 0;
    const qreal y1 = 0;
    const qreal x2 = hor ? size.width() : 0;
    const qreal y2 = ver ? size.height() : 0;

    QLinearGradient gradient( x1, y1, x2, y2 );
    gradient.setColorAt( 0.0, c1 );
    gradient.setColorAt( 1.0, c2 );

    QPainter p( &img );
    p.fillRect( 0, 0, size.width(), size.height(), QBrush( gradient ) );

    return img;
}

QBrush PaintingHelper::BuildGridBrush( QSize const& size )
{
    QImage img( size.width() * 2, size.height() * 2, QImage::Format_ARGB32 );
    img.fill( Qt::white );
    
    QPainter p( &img );
    p.fillRect( QRect( 0, 0, size.width(), size.height() ), Qt::lightGray );
    p.fillRect( QRect( size.width(), size.height(), size.width(), size.height() ), Qt::lightGray );

    return QBrush( img );
}

QImage PaintingHelper::BuildArrowIcon( QSize const& size, Qt::Edge dimension, const QColor& color )
{
    QImage img( size, QImage::Format_ARGB32 );
    img.fill( Qt::transparent );

    /*
    #
    ###
    #####
    ###
    #
    */
    QPainterPath path;
    path.moveTo( 0, 0 );
    path.lineTo( 0, size.height() - 1 );
    path.lineTo( size.width() - 1, size.height() / 2 );
    path.lineTo( 0, 0 );

    QMatrix rm;
    switch ( dimension )
    {
    case Qt::TopEdge:
        rm.rotate( 90 );
        break;
    case Qt::LeftEdge:
        rm.rotate( 0 );
        break;
    case Qt::RightEdge:
        rm.rotate( 180 );
        break;
    case Qt::BottomEdge:
        rm.rotate( 270 );
        break;
    default:
        return img;
    }

    QImage tmp( size, QImage::Format_ARGB32 );
    {
        QPainter p( &tmp );
        tmp.fill( Qt::transparent );
        p.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true );
        p.fillPath( path, color );
        //p.setPen( Qt::red );
        //p.drawLine( 0, 0, 0, size.height() - 1 );
        //p.setPen( Qt::yellow );
        //p.drawLine( 0, 0, size.width() - 1, size.height() / 2 );
        //p.setPen( Qt::magenta );
        //p.drawLine( 0, size.height() - 1, size.width() - 1, size.height() / 2 );
    }

    QPainter p( &img );
    p.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true );
    p.drawPixmap( 0, 0, QPixmap::fromImage( tmp ).transformed( rm ) );

    return img;
}

int PaintingHelper::hue( QPoint const& pt, QSize const& size )
{
    return 360 - pt.x() * 360 / ( size.width() - 1 );
}

int PaintingHelper::sat( QPoint const& pt, QSize const& size )
{
    return 255 - pt.y() * 255 / ( size.height() - 1 );
}

int PaintingHelper::val( QPoint const& pt, QSize const& size )
{
    Q_UNUSED( pt );
    Q_UNUSED( size );

    return 240;
}
