#include "ColorWidget.h"
#include "ui_ColorWidget.h"

#include "AbstractColorPalette.h"
#include "HSVPaletteWidget.h"


ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ColorWidget())
{
    ui->setupUi(this);

    AddPalette( "HSV", new HSVPaletteWidget() );

    connect( ui->paletteCombo, SIGNAL( currentIndexChanged( int ) ), SLOT( onPaletteType() ) );
}

ColorWidget::~ColorWidget()
{
}

void ColorWidget::AddPalette( QString const& name, AbstractColorPalette* pal )
{
    ui->paletteCombo->addItem( name, name );
    ui->paletteStack->addWidget( pal );
    paletteMap[name] = pal;
}

void ColorWidget::onPaletteType()
{
    const int idx = ui->paletteCombo->currentIndex();
    //if ( idx < 0 )
    //    return;

    const QString& key = ui->paletteCombo->itemData( idx ).toString();
    ui->paletteStack->setCurrentWidget( paletteMap[key] );
}
