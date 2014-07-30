#include "MouseHelper.h"

#include <QMouseEvent>


MouseHelper::MouseHelper( QWidget *_w )
    : QObject( _w )
    , w( _w )
    , isHover( false )
    , isPressed( false )
{
    Q_ASSERT( w );
    w->installEventFilter( this );
    w->setMouseTracking( true );
}

MouseHelper::~MouseHelper()
{
}

bool MouseHelper::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == w )
    {
        switch ( e->type() )
        {
        case QEvent::Enter:
            enterEvent( e );
            break;
        case QEvent::Leave:
            leaveEvent( e );
            break;
        case QEvent::MouseMove:
            mouseMoveEvent( static_cast< QMouseEvent * >( e ) );
            break;
        case QEvent::MouseButtonPress:
            mousePressEvent( static_cast< QMouseEvent * >( e ) );
            break;
        case QEvent::MouseButtonRelease:
            mouseReleaseEvent( static_cast< QMouseEvent * >( e ) );
            break;

        default:
            break;
        }
    }

    return QObject::eventFilter( obj, e );
}

void MouseHelper::enterEvent( QEvent* event )
{
    isHover = true;
}

void MouseHelper::leaveEvent( QEvent* event )
{
    isHover = false;
}

void MouseHelper::mouseMoveEvent( QMouseEvent* event )
{
    pos = event->pos();
}

void MouseHelper::mousePressEvent( QMouseEvent* event )
{
    isPressed = true;
}

void MouseHelper::mouseReleaseEvent( QMouseEvent* event )
{
    isPressed = false;
}
