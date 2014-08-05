#include "dropper.h"

#include "ColorPicker/ColorPicker.h"


Dropper::Dropper(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.test_1->SetColors( QColor( 255, 255, 0 ), QColor( 255, 0, 0 ) );
    ui.test_1->SetDimensions( Qt::BottomEdge );
    ui.test_1->SetOrientation( Qt::Horizontal );

    ui.test_2->SetColors( QColor( 0, 255, 128, 255 ), QColor( 0, 255, 128, 0 ) );
    ui.test_2->SetDimensions( Qt::LeftEdge | Qt::RightEdge );
    ui.test_2->SetOrientation( Qt::Vertical );

    ui.test_3->setColor( Qt::yellow );

    ui.test_4->setColorOld( QColor( 255, 0, 255, 100 ) );
    ui.test_4->setColorNew( QColor( 255, 255, 100, 50 ) );

    connect( ui.btn, SIGNAL( clicked() ), SLOT( showCP() ) );
}

Dropper::~Dropper()
{

}

void Dropper::showCP()
{
    ColorPicker *cp = new ColorPicker( this );
    cp->setWindowFlags( Qt::Window );
    cp->setAttribute( Qt::WA_DeleteOnClose, true );

    cp->show();

    cp->SetColor( QColor( 90, 150, 230, 120 ) );
}