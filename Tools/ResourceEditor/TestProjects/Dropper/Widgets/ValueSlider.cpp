#include "ValueSlider.h"

#include <QPainter>
#include <QApplication>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QStyleOption>

#include "../Helpers/MouseHelper.h"


namespace
{
    const QString editorQss =
        "border: 1px solid black;\n"
        "background: transparent;"
        "padding: 0px;"
        "margin: 0px;"
        ;
    const int digits = 8;
}


ValueSlider::ValueSlider(QWidget *parent)
    : QWidget( parent )
    , minVal( 0 )
    , maxVal( 1 )
    , val( 0.2 )
    , clickVal( 0 )
    , mouse( new MouseHelper( this ) )
{
    connect( mouse, SIGNAL( mousePress( const QPoint& ) ), SLOT( OnMousePress( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( OnMouseMove( const QPoint& ) ) );
    connect( mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnMouseRelease( const QPoint& ) ) );
    connect( mouse, SIGNAL( clicked() ), SLOT( OnMouseClick() ) );
}

ValueSlider::~ValueSlider()
{
}

void ValueSlider::DrawBackground( QPainter* p ) const
{
    const QRect rc = PosArea().adjusted( 0, 0, -1, -1 );
    
    p->fillRect( rc, Qt::gray );
    p->setPen( QPen( Qt::black, 1 ) );
    p->drawRect( rc );
}

void ValueSlider::DrawForeground( QPainter* p ) const
{
    const double relVal = ( val - minVal ) / ( maxVal - minVal );
    const int w = int( ( PosArea().width() - 2 ) * relVal );
    const QRect rc( 1, 1, w, PosArea().height() - 2 );
    const QRect clip = PosArea().adjusted( 1, 1, -2, -1 );

    const QColor c1( 235, 235, 235 );
    const QColor c2( 200, 200, 200 );
    const QColor c3( 235, 235, 235 );

    QLinearGradient gradient( 1, 1, 1, rc.height() );
    gradient.setColorAt( 0.0, c1 );
    gradient.setColorAt( 0.8, c2 );
    gradient.setColorAt( 1.0, c3 );

    p->setClipRect( clip );
    p->fillRect( rc, gradient );

    if ( !IsEditorMode() )
    {
        QStyleOptionFrameV2 panel;
        panel.initFrom( this );
        style()->drawItemText( p, clip.adjusted( 3, 0, 0, 0 ), (Qt::AlignLeft | Qt::AlignVCenter), palette(), true, QString::number( val, 'g', digits ), QPalette::WindowText );
    }
}

QRect ValueSlider::PosArea() const
{
    return QRect( 0, 0, width(), height() );
}

void ValueSlider::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QPainter p( this );

    p.save();
    DrawBackground( &p );
    p.restore();
    p.save();
    DrawForeground( &p );
    p.restore();
}

void ValueSlider::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
}

bool ValueSlider::eventFilter( QObject* obj, QEvent* e )
{
    return QWidget::eventFilter( obj, e );
}

bool ValueSlider::IsEditorMode() const
{
    return !editor.isNull();
}

void ValueSlider::OnMousePress( const QPoint& pos )
{
    clickPos = pos;
    clickVal = val;

    QApplication::setOverrideCursor( Qt::BlankCursor );
}

void ValueSlider::OnMouseMove( const QPoint& pos )
{
    if ( mouse->IsPressed() )
    {
        const int dist = pos.x() - clickPos.x();
        const int w = width();
        const double dd = double( dist ) / double( w );
        const double dw = ( maxVal - minVal );
        const double ofs = dd * dw;

        val = clickVal + ofs;
        if ( val < minVal )
            val = minVal;
        if ( val > maxVal )
            val = maxVal;

        repaint();
    }
}

void ValueSlider::OnMouseRelease( const QPoint& pos )
{
    QCursor::setPos( mapToGlobal( clickPos ) );
    QApplication::restoreOverrideCursor();
}

void ValueSlider::OnMouseClick()
{
    if ( editor )
        delete editor;
    QDoubleValidator *validator = new QDoubleValidator( minVal, maxVal, digits );
    editor = new QLineEdit(this);
    editor->setValidator( validator );
    editor->setStyleSheet( editorQss );
    editor->move( 0, 0 );
    editor->resize( width(), height() );

    editor->setText( QString::number( val, 'g', digits ) );

    editor->show();
    editor->setFocus();
    update();
}
