#include "ColorPicker.h"
#include "ui_ColorPicker.h"

#include "AbstractColorPicker.h"
#include "ColorPickerHSV.h"
#include "ColorPickerRGBAM.h"
#include "ColorPreview.h"


ColorPicker::ColorPicker(QWidget *parent)
    : AbstractColorPicker( parent )
    , ui( new Ui::ColorPicker() )
{
    ui->setupUi( this );

    setFocusPolicy( Qt::ClickFocus );

    // Pickers
    RegisterPicker( "HSV rectangle", new ColorPickerHSV() );

    // Editors
    RegisterColorSpace( "RGBA M", new ColorPickerRGBAM() );

    // Preview
    connect( this, SIGNAL( changing( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ) );
    connect( this, SIGNAL( changed( const QColor& ) ), ui->preview, SLOT( SetColorNew( const QColor& ) ) );
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
    ConnectPicker( picker );
}

void ColorPicker::RegisterColorSpace( QString const& key, AbstractColorPicker* picker )
{
    delete colorSpaces[key];
    colorSpaces[key] = picker;

    ui->colorSpaceCombo->addItem( key, key );
    ui->colorSpaceStack->addWidget( picker );
    ConnectPicker( picker );
}

void ColorPicker::SetColorInternal( const QColor& c )
{
    UpdateControls( c );
    ui->preview->SetColorOld( c );
    ui->preview->SetColorNew( c );
}

void ColorPicker::OnChanging( const QColor& c )
{
    AbstractColorPicker *source = qobject_cast<AbstractColorPicker *>( sender() );
    UpdateControls( c, source );
    emit changing( c );
}

void ColorPicker::OnChanged( const QColor& c )
{
    AbstractColorPicker *source = qobject_cast<AbstractColorPicker *>( sender() );
    UpdateControls( c, source );
    emit changed( c );
}

void ColorPicker::UpdateControls( const QColor& c, AbstractColorPicker* source )
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

void ColorPicker::ConnectPicker( AbstractColorPicker* picker )
{
    connect( picker, SIGNAL( begin() ), SIGNAL( begin() ) );
    connect( picker, SIGNAL( changing( const QColor& ) ), SLOT( OnChanging( const QColor& ) ) );
    connect( picker, SIGNAL( changed( const QColor& ) ), SLOT( OnChanged( const QColor& ) ) );
    connect( picker, SIGNAL( canceled() ), SIGNAL( canceled() ) );
}
