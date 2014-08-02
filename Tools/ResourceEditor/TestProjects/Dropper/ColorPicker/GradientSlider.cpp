#include "GradientSlider.h"

#include <QPainter>

#include "../Helpers/PaintingHelper.h"


GradientSlider::GradientSlider(QWidget *parent)
    : AbstractSlider(parent)
    , arrowSize( 9, 9 )
    , orientation( Qt::Horizontal )
{
}

GradientSlider::~GradientSlider()
{
}

void GradientSlider::SetColors( const QColor& _c1, const QColor& _c2 )
{
    c1 = _c1;
    c2 = _c2;
    InvalidateBackground();
}

void GradientSlider::SetDimensions( const Qt::Edges& flags )
{
    arrows = flags;
    InvalidateForeGround();
}

void GradientSlider::SetOrientation( Qt::Orientation _orientation )
{
    orientation = _orientation;
    InvalidateBackground();
}

QPixmap GradientSlider::DrawBackground() const
{
    const QRect& rc = PosArea();
    const QImage& bg = PaintingHelper::BuildGradient( rc.size(), c1, c2, orientation );

    QPixmap pix( size() );
    pix.fill( Qt::transparent );
    QPainter p( &pix );

    const QBrush& br = PaintingHelper::BuildGridBrush( QSize( 5, 5 ) );
    p.fillRect( rc, br );
    p.drawImage( rc.topLeft(), bg );

    return pix;
}

QPixmap GradientSlider::DrawForground() const
{
    Qt::Edges flags;
    switch ( orientation )
    {
    case Qt::Horizontal:
        flags = arrows & ( Qt::TopEdge | Qt::BottomEdge );
        break;
    case Qt::Vertical:
        flags = arrows & ( Qt::LeftEdge | Qt::RightEdge );
        break;
    }

    QPixmap buf( size() );
    buf.fill( Qt::transparent );

    QPainter p( &buf );
    if ( flags.testFlag( Qt::TopEdge ) )
        drawArrow( Qt::TopEdge, &p );
    if ( flags.testFlag( Qt::LeftEdge ) )
        drawArrow( Qt::LeftEdge, &p );
    if ( flags.testFlag( Qt::RightEdge ) )
        drawArrow( Qt::RightEdge, &p );
    if ( flags.testFlag( Qt::BottomEdge ) )
        drawArrow( Qt::BottomEdge, &p );

    return buf;
}

void GradientSlider::drawArrow( Qt::Edge arrow, QPainter *p ) const
{
    const auto it = arrowCache.constFind( arrow );
    if ( it == arrowCache.constEnd() )
    {
        arrowCache[arrow] = QPixmap::fromImage( PaintingHelper::BuildArrowIcon( arrowSize, arrow, Qt::black ) );
    }

    const QPoint& currentPos = Pos();
    QPoint pos;

    switch ( arrow )
    {
    case Qt::TopEdge:
        pos.setX( currentPos.x() - arrowSize.width() / 2 );
        pos.setY( 0 );
        break;
    case Qt::LeftEdge:
        pos.setX( 0 );
        pos.setY( currentPos.y() - arrowSize.height() / 2 );
        break;
    case Qt::RightEdge:
        pos.setX( width() - arrowSize.width() + 1 );
        pos.setY( currentPos.y() - arrowSize.height() / 2 );
        break;
    case Qt::BottomEdge:
        pos.setX( currentPos.x() - arrowSize.width() / 2 );
        pos.setY( height() - arrowSize.height() + 1 );
        break;
    default:
        return;
    }

    QRect rc( pos, arrowSize );
    p->drawPixmap( pos, arrowCache[arrow] );
}
