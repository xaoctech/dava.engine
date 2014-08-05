#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#include "AbstractColorPicker.h"
#include "ColorPickerHSV.h"
#include "ColorPickerRGBAM.h"


ColorPicker::ColorPicker(QWidget *parent)
    : AbstractColorPicker( parent )
    , ui( new Ui::ColorPicker() )
{
    ui->setupUi( this );

    // Pickers
    RegisterPicker( "HSV rectangle", new ColorPickerHSV() );

    // Editors
    RegisterColorSpace( "RGBA M", new ColorPickerRGBAM() );
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

void ColorPicker::RegisterColorSpace( QString const& key, AbstractColorPicker* picker )
{
    delete colorSpaces[key];
    colorSpaces[key] = picker;

    ui->colorSpaceCombo->addItem( key, key );
    ui->colorSpaceStack->addWidget( picker );
}

void ColorPicker::SetColorInternal( QColor const& c )
{
    UpdateControls( c );
}

void ColorPicker::OnChanging( QColor const& c )
{
    AbstractColorPicker *source = qobject_cast<AbstractColorPicker *>( sender() );
    UpdateControls( c, source );
    emit changing( c );
}

void ColorPicker::OnChanged( QColor const& c )
{
    AbstractColorPicker *source = qobject_cast<AbstractColorPicker *>( sender() );
    UpdateControls( c, source );
    emit changed( c );
}

void ColorPicker::UpdateControls( QColor const& c, AbstractColorPicker* source )
{
    for ( auto it = pickers.begin(); it != pickers.end(); ++it )
    {
        AbstractColorPicker *recv = it.value();
        if ( recv && recv != source )
        {
            recv->SetColor( c );
        }
    }
    for ( auto it = colorSpaces.begin(); it != colorSpaces.end(); ++it )
    {
        AbstractColorPicker *recv = it.value();
        if ( recv && recv != source )
        {
            recv->SetColor( c );
        }
    }
}
