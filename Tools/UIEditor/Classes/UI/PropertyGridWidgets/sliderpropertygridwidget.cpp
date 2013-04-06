#include "sliderpropertygridwidget.h"
#include "ui_sliderpropertygridwidget.h"

#include "PropertyNames.h"
#include "BackgroundGridWidgetHelper.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"
#include "WidgetSignalsBlocker.h"
#include "ResourcesManageHelper.h"
#include "TexturePacker/ResourcePacker2D.h"

#include <QFileDialog>

using namespace PropertyNames;

static const QString SLIDER_PROPERTY_BLOCK_NAME = "Slider options";

SliderPropertyGridWidget::SliderPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::SliderPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(SLIDER_PROPERTY_BLOCK_NAME);
	ConnectToSignals();
	
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

SliderPropertyGridWidget::~SliderPropertyGridWidget()
{
    delete ui;
}

void SliderPropertyGridWidget::ConnectToSignals()
{
    // Create connection between slider and double spinbox
    connect(ui->valuePosition, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
    // Add sprite handlers
    connect(ui->openThumbSpriteButton, SIGNAL(clicked()), this, SLOT(OnOpenSpriteDialog()));
    connect(ui->openMinSpriteButton, SIGNAL(clicked()), this, SLOT(OnOpenSpriteDialog()));
    connect(ui->openMaxSpriteButton, SIGNAL(clicked()), this, SLOT(OnOpenSpriteDialog()));
    // Remove sprite handlers
    connect(ui->removeThumbSpriteButton, SIGNAL(clicked()), this, SLOT(OnRemoveSprite()));
    connect(ui->removeMinSpriteButton, SIGNAL(clicked()), this, SLOT(OnRemoveSprite()));
    connect(ui->removeMaxSpriteButton, SIGNAL(clicked()), this, SLOT(OnRemoveSprite()));
}

void SliderPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	
	FillComboboxes();

    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, SLIDER_VALUE_PROPERTY_NAME, ui->valueSpin);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, SLIDER_MIN_VALUE_PROPERTY_NAME, ui->minValueSpin);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, SLIDER_MAX_VALUE_PROPERTY_NAME, ui->maxValueSpin);
	// Sprite line edit
	RegisterLineEditWidgetForProperty(propertiesMap, SLIDER_THUMB_SPRITE_PROPERTY_NAME, ui->thumbSpriteLineEdit);
	RegisterLineEditWidgetForProperty(propertiesMap, SLIDER_MIN_SPRITE_PROPERTY_NAME, ui->minSpriteLineEdit);
	RegisterLineEditWidgetForProperty(propertiesMap, SLIDER_MAX_SPRITE_PROPERTY_NAME, ui->maxSpriteLineEdit);
	// Frame spin boxes
	RegisterSpinBoxWidgetForProperty(propertiesMap, SLIDER_THUMB_SPRITE_FRAME_PROPERTY_NAME, ui->thumbFrameSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, SLIDER_MIN_SPRITE_FRAME_PROPERTY_NAME, ui->minFrameSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, SLIDER_MAX_SPRITE_FRAME_PROPERTY_NAME, ui->maxFrameSpinBox);
	// Draw type combo
	RegisterComboBoxWidgetForProperty(propertiesMap, SLIDER_MIN_DRAW_TYPE_PROPERTY_NAME, ui->minDrawTypeComboBox);
	RegisterComboBoxWidgetForProperty(propertiesMap, SLIDER_MAX_DRAW_TYPE_PROPERTY_NAME, ui->maxDrawTypeComboBox);
}

void SliderPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
	UnregisterDoubleSpinBoxWidget(ui->valueSpin);
    UnregisterDoubleSpinBoxWidget(ui->minValueSpin);
    UnregisterDoubleSpinBoxWidget(ui->maxValueSpin);
	UnregisterLineEditWidget(ui->thumbSpriteLineEdit);
	UnregisterLineEditWidget(ui->minSpriteLineEdit);
	UnregisterLineEditWidget(ui->maxSpriteLineEdit);
	UnregisterSpinBoxWidget(ui->thumbFrameSpinBox);
	UnregisterSpinBoxWidget(ui->minFrameSpinBox);
	UnregisterSpinBoxWidget(ui->maxFrameSpinBox);
	UnregisterComboBoxWidget(ui->minDrawTypeComboBox);
	UnregisterComboBoxWidget(ui->maxDrawTypeComboBox);
}

void SliderPropertyGridWidget::FillComboboxes()
{
    WidgetSignalsBlocker minDrawTypeBlocker(ui->minDrawTypeComboBox);
 	WidgetSignalsBlocker maxDrawTypeBlocker(ui->maxDrawTypeComboBox);

    ui->minDrawTypeComboBox->clear();
	ui->maxDrawTypeComboBox->clear();
    int itemsCount = BackgroundGridWidgetHelper::GetDrawTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->minDrawTypeComboBox->addItem(BackgroundGridWidgetHelper::GetDrawTypeDesc(i));
		ui->maxDrawTypeComboBox->addItem(BackgroundGridWidgetHelper::GetDrawTypeDesc(i));
    }
}


void SliderPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget,
														const PROPERTYGRIDWIDGETSITER &iter, const double value)
{
    if (doubleSpinBoxWidget == NULL)
    {
        Logger::Error("SliderPropertyGridWidget::ProcessDoubleSpinBoxValueChanged: senderWidget is NULL!");
        return;
    }

    // Try to process this control-specific widgets.	
	if ((doubleSpinBoxWidget == ui->valueSpin) ||
		(doubleSpinBoxWidget == ui->maxValueSpin) || (doubleSpinBoxWidget == ui->minValueSpin))
	{
		// Don't update the property if the text wasn't actually changed.
		double curValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata, iter->second.getProperty().name());
		if (curValue == value)
		{
			return;
		}

		BaseCommand* command = new ChangePropertyCommand<double>(activeMetadata, iter->second, value);
  	  	CommandsController::Instance()->ExecuteCommand(command);
   		SafeRelease(command);
		return;
	}

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessDoubleSpinBoxValueChanged(doubleSpinBoxWidget, iter, value);
}

void SliderPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
																			const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
	double propertyValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                 					     curProperty.name(),
                                                     					 isPropertyValueDiffers);
    WidgetSignalsBlocker blocker(doubleSpinBoxWidget);
    doubleSpinBoxWidget->setValue(propertyValue);
	UpdateWidgetPalette(doubleSpinBoxWidget, curProperty.name());
	// Update related properties for concrete spin box
	if (doubleSpinBoxWidget == ui->valueSpin)
	{
		//Upate value of slider
		ui->valuePosition->setValue((int)propertyValue);		
		return;
    }
	else if (doubleSpinBoxWidget == ui->maxValueSpin) 
	{
		//Update max value
		ui->valuePosition->setMaximum((int)propertyValue);
		ui->valueSpin->setMaximum(propertyValue);
		return;
	}
	else if (doubleSpinBoxWidget == ui->minValueSpin)
	{
		//Update min value
		ui->valuePosition->setMinimum((int)propertyValue);
		ui->valueSpin->setMinimum(propertyValue);
		return;
	}
	
    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(doubleSpinBoxWidget, curProperty);
}

void SliderPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                         const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("SliderPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();
    
    if ((senderWidget == ui->minDrawTypeComboBox) || (ui->maxDrawTypeComboBox))
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetDrawType(selectedIndex));
    }

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void SliderPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
{
	// Don't update the property if the value wasn't actually changed.
    int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}

    BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void SliderPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
    if ((comboBoxWidget == ui->minDrawTypeComboBox) || (comboBoxWidget == ui->maxDrawTypeComboBox))
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetDrawTypeDescByType((UIControlBackground::eDrawType)propertyValue));
    }

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}

void SliderPropertyGridWidget::OnOpenSpriteDialog()
{
    QPushButton* senderWidget = dynamic_cast<QPushButton*>(QObject::sender());

    if (senderWidget == NULL)
        return;
		
	// Pack all available sprites each time user open sprite dialog
	ResourcePacker2D *resPacker = new ResourcePacker2D();
	resPacker->InitFolders(ResourcesManageHelper::GetSpritesDatasourceDirectory().toStdString(),
	 					 				ResourcesManageHelper::GetSpritesDirectory().toStdString());
	resPacker->PackResources();
    
	// Setup default sprites path
	QString currentSpriteDir = GetSpritePathForButton(senderWidget);
		
    QString spriteName = QFileDialog::getOpenFileName( this, tr( "Choose a sprite file file" ),
															currentSpriteDir,
															tr( "Sprites (*.txt)" ) );
    // Exit if sprite name is empty
    if( spriteName.isNull() || spriteName.isEmpty())
        return;
	// Convert file path into Unix-style path
	spriteName = ResourcesManageHelper::ConvertPathToUnixStyle(spriteName);

	if (ResourcesManageHelper::ValidateResourcePath(spriteName))
	{
		// Sprite name should be pre-processed to use relative path.
		QString processedSpriteName = ResourcesManageHelper::GetResourceRelativePath(spriteName);
		SetSprite(senderWidget, processedSpriteName);
	}
	else
	{
		ResourcesManageHelper::ShowErrorMessage(spriteName);
	}
    
    SafeDelete(resPacker);
}

void SliderPropertyGridWidget::OnRemoveSprite()
{
    QPushButton* senderWidget = dynamic_cast<QPushButton*>(QObject::sender());

    if (senderWidget == NULL)
        return;

	//When we pass empty spriteLineEdit to command - this will cause removal of sprite
	SetSprite(senderWidget, "");
}

void SliderPropertyGridWidget::SetSprite(QWidget *senderWidget, const QString& spritePath)
{
    if (senderWidget == NULL)
        return;
		
    if (senderWidget == ui->openThumbSpriteButton)
    {
        WidgetSignalsBlocker blocker(ui->thumbSpriteLineEdit);
        ui->thumbSpriteLineEdit->setText(spritePath);
        HandleLineEditEditingFinished(ui->thumbSpriteLineEdit);
    }
    else if (senderWidget == ui->openMinSpriteButton)
    {
        WidgetSignalsBlocker blocker(ui->minSpriteLineEdit);     
        ui->minSpriteLineEdit->setText(spritePath);
        HandleLineEditEditingFinished(ui->minSpriteLineEdit);
    }
    else if (senderWidget == ui->openMaxSpriteButton)
    {
        WidgetSignalsBlocker blocker(ui->maxSpriteLineEdit);       
        ui->maxSpriteLineEdit->setText(spritePath);
        HandleLineEditEditingFinished(ui->maxSpriteLineEdit);
    }
}

QString SliderPropertyGridWidget::GetSpritePathForButton(QWidget *senderWidget)
{
	QString spriteDirectory = ResourcesManageHelper::GetSpritesDirectory();
	// Get current sprite path that corresponds to pushed button and setup directory to open
	if (senderWidget == ui->openThumbSpriteButton)
    {
		spriteDirectory = ResourcesManageHelper::GetDefaultSpritesPath(this->ui->thumbSpriteLineEdit->text());
    }
    else if (senderWidget == ui->openMinSpriteButton)
    {
		spriteDirectory = ResourcesManageHelper::GetDefaultSpritesPath(this->ui->minSpriteLineEdit->text());
    }
    else if (senderWidget == ui->openMaxSpriteButton)
    {
		spriteDirectory = ResourcesManageHelper::GetDefaultSpritesPath(this->ui->maxSpriteLineEdit->text());
	}
	
	return spriteDirectory;
}

void SliderPropertyGridWidget::OnSliderValueChanged(int value)
{
	ui->valueSpin->setValue(value);
}