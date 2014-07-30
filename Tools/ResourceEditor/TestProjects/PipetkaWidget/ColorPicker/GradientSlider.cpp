#include "GradientSlider.h"

#include <QMouseEvent>
#include <QPainter>


GradientSlider::GradientSlider(QWidget *parent)
    : GradientWidget(parent)
    , arrowSize( 5, 5 )
    , mouse( new MouseHelper( this ) )
{
}

GradientSlider::~GradientSlider()
{
}

void GradientSlider::setEditorDimensions( Qt::Edges flags )
{
    arrows = flags;
    repaint();
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

    return buf;
}

void GradientSlider::onMousePress()
{
}

void GradientSlider::onMouseRelease()
{
}
