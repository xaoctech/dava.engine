#include "ColorPickerRGBAM.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "ColorComponentSlider.h"
#include "../Helpers/PaintingHelper.h"


ColorPickerRGBAM::ColorPickerRGBAM(QWidget *parent)
    : AbstractColorPicker(parent)
{
    r = new ColorComponentSlider();
    g = new ColorComponentSlider();
    b = new ColorComponentSlider();
    a = new ColorComponentSlider();
    m = new ColorComponentSlider();

    ColorComponentSlider *sliders[] = { r, g, b, a, m, };
    const QString labels[] = { "Red", "Green", "Blue", "Alpha", "Multiply" };
    
    QVBoxLayout *l = new QVBoxLayout();
    l->setMargin( 0 );
    l->setContentsMargins( 0, 0, 0, 0 );

    for ( int i = 0; i < sizeof( sliders ) / sizeof( *sliders ); i++ )
    {
        l->addLayout( CreateSlider( labels[i], sliders[i] ) );
    }
    l->addStretch( 1 );

    setLayout( l );

    SetColor( Qt::lightGray );
}

ColorPickerRGBAM::~ColorPickerRGBAM()
{
}

void ColorPickerRGBAM::SetColorInternal( const QColor& c )
{
    r->SetColorRange( PaintingHelper::MinColorComponent( c, 'r' ), PaintingHelper::MaxColorComponent( c, 'r' ) );
    g->SetColorRange( PaintingHelper::MinColorComponent( c, 'g' ), PaintingHelper::MaxColorComponent( c, 'g' ) );
    b->SetColorRange( PaintingHelper::MinColorComponent( c, 'b' ), PaintingHelper::MaxColorComponent( c, 'b' ) );
    a->SetColorRange( PaintingHelper::MinColorComponent( c, 'a' ), PaintingHelper::MaxColorComponent( c, 'a' ) );
}

QLayout* ColorPickerRGBAM::CreateSlider( const QString& text, ColorComponentSlider *w ) const
{
    QHBoxLayout *l = new QHBoxLayout();
    QLabel *txt = new QLabel( text );
    txt->setMinimumSize( 40, 0 );
    txt->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
    l->addWidget( txt );
    l->addWidget( w );

    return l;
}