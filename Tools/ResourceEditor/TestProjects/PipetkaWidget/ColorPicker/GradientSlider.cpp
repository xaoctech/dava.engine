#include "GradientSlider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include <QDebug>


GradientSlider::GradientSlider(QWidget *parent)
    : GradientWidget(parent)
    , arrowSize( 20, 20 )
    , mouse( new MouseHelper( this ) )
{
    connect( mouse, SIGNAL( mousePress( const QPoint& ) ), SLOT( onMousePress( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( onMouseMove( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( onMouseRelease( const QPoint& ) ) );
}

GradientSlider::~GradientSlider()
{
}

void GradientSlider::setEditorDimensions( Qt::Edges flags )
{
    arrows = flags;
    update();
}

QColor GradientSlider::GetColor() const
{
    return QColor();
}

void GradientSlider::setColor( QColor const& c )
{
}

QPixmap GradientSlider::drawContent() const
{
    QPixmap buf( size() );
    buf.fill( Qt::transparent );

    QPainter p( &buf );
    drawArrows( &p );

    return buf;
}

void GradientSlider::onMousePress( const QPoint& pos )
{
    startColor = GetColor();
    setPos( pos );
    emit begin();
}

void GradientSlider::onMouseMove( QPoint const& pos )
{
    if ( mouse->IsPressed() )
    {
        setPos( pos );
        emit changing( GetColor() );
    }
}

void GradientSlider::onMouseRelease( const QPoint& pos )
{
    setPos( pos );
    const QColor endColor = GetColor();
    if ( startColor != endColor )
    {
        emit changed( endColor );
    }
    else
    {
        emit canceled();
    }
}

QPoint GradientSlider::fitInGradient( QPoint const& pos ) const
{
    const QRect rc = QRect( 0, 0, width(), height() ).adjusted( padding().left, padding().top, -padding().right, -padding().bottom );
    QPoint pt = pos;

    if ( !rc.contains( pt ) )
    {
        if ( pt.x() < rc.left() )
        {
            pt.setX( rc.left() );
        }
        if ( pt.x() > rc.right() )
        {
            pt.setX( rc.right() );
        }
        if ( pt.y() < rc.top() )
        {
            pt.setY( rc.top() );
        }
        if ( pt.y() > rc.bottom() )
        {
            pt.setY( rc.bottom() );
        }
    }

    return pt;
}

void GradientSlider::setPos( QPoint const& pos )
{
    currentPos = fitInGradient( pos );
    repaint();
}

void GradientSlider::drawArrows( QPainter* p ) const
{
    p->save();

    if ( arrows.testFlag( Qt::TopEdge ) )
        drawArrow( Qt::TopEdge, p );
    if ( arrows.testFlag( Qt::LeftEdge ) )
        drawArrow( Qt::LeftEdge, p );
    if ( arrows.testFlag( Qt::RightEdge ) )
        drawArrow( Qt::RightEdge, p );
    if ( arrows.testFlag( Qt::BottomEdge ) )
        drawArrow( Qt::BottomEdge, p );

    p->restore();
}

void GradientSlider::drawArrow( Qt::Edge arrow, QPainter *p ) const
{
    QStyle::PrimitiveElement pe;
    QPoint pos;

    switch ( arrow )
    {
    case Qt::TopEdge:
        pe = QStyle::PE_IndicatorArrowDown;
        pos.setX( currentPos.x() - arrowSize.width() / 2 );
        pos.setY( 0 );
        break;
    case Qt::LeftEdge:
        pe = QStyle::PE_IndicatorArrowRight;
        pos.setX( 0 );
        pos.setY( currentPos.y() - arrowSize.height() / 2 );
        break;
    case Qt::RightEdge:
        pe = QStyle::PE_IndicatorArrowLeft;
        pos.setX( width() - arrowSize.width() - 1 );
        pos.setY( currentPos.y() - arrowSize.height() / 2 );
        break;
    case Qt::BottomEdge:
        pe = QStyle::PE_IndicatorArrowUp;
        pos.setX( currentPos.x() - arrowSize.width() / 2 );
        pos.setY( height() - arrowSize.height() - 1 );
        break;
    default:
        return;
    }

    QStyleOption opt;
    opt.initFrom( this );
    opt.rect = QRect( pos, arrowSize );
    p->fillRect( opt.rect, Qt::red );
    style()->drawPrimitive( pe, &opt, p, this );
}
