#include "PaletteHSV.h"

#include <QPainter>

#include "../Helpers/PaintingHelper.h"


PaletteHSV::PaletteHSV(QWidget *parent)
    : AbstractSlider(parent)
    , cursorSize( 8, 8 )
{
}

PaletteHSV::~PaletteHSV()
{
}

int PaletteHSV::GetHue() const
{
    const int h = PaintingHelper::HueRC( Pos(), size() );
    return h;
}

int PaletteHSV::GetSat() const
{
    const int s = PaintingHelper::SatRC( Pos(), size() );
    return s;
}

void PaletteHSV::setColor( int hue, int sat )
{
    QColor c;
    c.setHsv( hue, sat, PaintingHelper::ValRC( Pos(), size() ) );
    const QPoint pt = PaintingHelper::GetHSVColorPoint( c, size() );
    SetPos( pt );
}

void PaletteHSV::setColor( const QColor& c )
{
    setColor( c.hue(), c.saturation() );
}

QPixmap PaletteHSV::DrawBackground() const
{
    const QImage& pal = PaintingHelper::BuildHSVImage( size() );
    return QPixmap::fromImage( pal );
}

QPixmap PaletteHSV::DrawForground() const
{
    QPixmap pix( size() );
    pix.fill( Qt::transparent );

    QPainter p( &pix );
    DrawCursor( &p );

    return pix;
}

void PaletteHSV::DrawCursor( QPainter* p ) const
{
    const int sx = cursorSize.width() / 2;
    const int sy = cursorSize.height() / 2;
    QRect rc( QPoint( Pos().x() - sx, Pos().y() - sy ), cursorSize );

    const int h = GetHue();
    const int s = GetSat();
    const int v = PaintingHelper::ValRC( Pos(), size() );
    QColor c;
    c.setHsv( h, s, v );
    p->fillRect( rc, c );

    p->setPen( Qt::white );
    p->drawRect( rc );
    rc.adjust( -1, -1, 1, 1 );
    p->setPen( Qt::black );
    p->drawRect( rc );
}
