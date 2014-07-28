#include "ColorValue.h"

#include <QPainter>


ColorValue::ColorValue(QWidget *parent)
    : QFrame(parent)
{
}

ColorValue::~ColorValue()
{
}

void ColorValue::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );
}

void ColorValue::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
    cache = QPixmap();
}
