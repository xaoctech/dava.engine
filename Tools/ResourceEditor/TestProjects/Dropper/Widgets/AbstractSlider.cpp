#include "AbstractSlider.h"

#include <QPainter>
#include <QResizeEvent>

#include "../Helpers/MouseHelper.h"


AbstractSlider::AbstractSlider(QWidget *parent)
    : QWidget(parent)
    , mouse( new MouseHelper( this ) )
{
    connect( mouse, SIGNAL( mousePress( const QPoint& ) ), SLOT( OnMousePress( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( OnMouseMove( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnMouseRelease( const QPoint& ) ) );
}

AbstractSlider::~AbstractSlider()
{
}

QPointF AbstractSlider::PosF() const
{
    const QRect& rc = PosArea();
    const qreal x = double( pos.x() ) / double( rc.width() );
    const qreal y = double( pos.y() ) / double( rc.height() );

    return QPointF( x, y );
}

void AbstractSlider::SetPostF( const QPointF& posF )
{
    const QRect& rc = PosArea();
    const int x = int( rc.width() * posF.x() );
    const int y = int( rc.height() * posF.y() );
    QPoint newPos( x, y );

    newPos += rc.topLeft();
    SetPos( newPos );
}

void AbstractSlider::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QPainter p( this );

    p.save();
    DrawBackground( &p );
    p.restore();
    p.save();
    DrawForground( &p );
    p.restore();
}

void AbstractSlider::resizeEvent( QResizeEvent* e )
{
    QPoint oldPos = pos;
    const int oldW = e->oldSize().width();
    const int oldH = e->oldSize().height();

    // Skip resize during initialization
    if ( oldW < 0 || oldH < 0 )
        return;

    const qreal xScale = oldW ? double( e->size().width() ) / double( oldW ) : 0;
    const qreal yScale = oldH ? double( e->size().height() ) / double( oldH ) : 0;

    pos.setX( int( oldPos.x() * xScale ) );
    pos.setY( int( oldPos.y() * yScale ) );

    update();
}

void AbstractSlider::DrawBackground( QPainter *p ) const
{
    Q_UNUSED( p );
}

void AbstractSlider::DrawForground( QPainter *p ) const
{
    Q_UNUSED( p );
}

QRect AbstractSlider::PosArea() const
{
    return QRect( 0, 0, width(), height() );
}

QPoint AbstractSlider::Pos() const
{
    return pos;
}

MouseHelper* AbstractSlider::Mouse() const
{
    return mouse;
}

void AbstractSlider::OnMousePress( const QPoint & _pos )
{
    SetPos( _pos );
    pressPos = pos;
    emit started( PosF() );
    emit changing( PosF() );
}

void AbstractSlider::OnMouseMove( const QPoint & _pos )
{
    if ( mouse->IsPressed() )
    {
        SetPos( _pos );
        emit changing( PosF() );
    }
}

void AbstractSlider::OnMouseRelease( const QPoint & _pos )
{
    SetPos( _pos );
    if ( pressPos != pos )
    {
        emit changed( PosF() );
    }
    else
    {
        emit canceled();
    }
}

void AbstractSlider::SetPos(const QPoint& _pos)
{
    const QRect &rc = PosArea();
    QPoint pt = _pos;

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

    pos = pt - rc.topLeft();
    repaint();
}
