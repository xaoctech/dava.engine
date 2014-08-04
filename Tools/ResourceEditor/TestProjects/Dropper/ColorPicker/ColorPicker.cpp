#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#include "AbstractColorPicker.h"
#include "ColorPickerHSV.h"


ColorPicker::ColorPicker(QWidget *parent)
    : QWidget(parent)
    , ui( new Ui::ColorPicker() )
{
    ui->setupUi( this );

    RegisterPicker( "HSV", new ColorPickerHSV() );
}

ColorPicker::~ColorPicker()
{
}

void ColorPicker::RegisterPicker( QString const& key, AbstractColorPicker* picker )
{
    delete pickers[key];
    pickers[key] = picker;

    ui->pickerCombo->addItem( key, key );
    ui->pickerStack->addWidget( picker );
}
