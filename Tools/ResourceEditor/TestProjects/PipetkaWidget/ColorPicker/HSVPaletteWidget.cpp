#include "HSVPaletteWidget.h"

#include <QPainter>

#include "PaletteHelper.h"


HSVPaletteWidget::HSVPaletteWidget(QWidget *parent)
    : QWidget(parent)
{
}

HSVPaletteWidget::~HSVPaletteWidget()
{

}

QColor HSVPaletteWidget::GetColor() const
{
    return color;
}

void HSVPaletteWidget::setColor( QColor const& c )
{
    if ( color != c )
    {
        color = c;
    }
}

void HSVPaletteWidget::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    if ( cache.isNull() )
    {
        const QImage& pal = PaletteHelper::BuildHSVImage( size() );
        cache = QPixmap::fromImage( pal );
    }

    QPainter p( this );
    p.drawPixmap( 0, 0, cache );
}


void HSVPaletteWidget::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
    cache = QPixmap();
}
