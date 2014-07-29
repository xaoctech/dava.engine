#include "GradientWidget.h"

#include <QPainter>
#include "PaletteHelper.h"


GradientWidget::GradientWidget(QWidget *parent)
    : QWidget(parent)
    , hor(true)
    , ver(false)
{
}

GradientWidget::~GradientWidget()
{
}

void GradientWidget::setColorRange( QColor const& start, QColor const& stop )
{
    startColor = start;
    stopColor = stop;
}

void GradientWidget::setDimensions( bool _hor, bool _ver )
{
    if ( hor != _hor || ver != _ver )
    {
        hor = _hor;
        ver = _ver;

        cache = QPixmap();
        repaint();
    }
}

void GradientWidget::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QWidget::paintEvent( e );

    if ( cache.isNull() )
    {
        const QImage& bg = PaletteHelper::BuildGradient( size(), startColor, stopColor, hor, ver );
        cache = QPixmap::fromImage( bg );
    }

    QPainter p( this );
    p.drawPixmap( 0, 0, cache );
}

void GradientWidget::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
    cache = QPixmap();
}
