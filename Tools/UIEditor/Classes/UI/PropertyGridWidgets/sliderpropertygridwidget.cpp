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
}

void SliderPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, SLIDER_VALUE_PROPERTY_NAME, ui->valueSpin);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, SLIDER_MIN_VALUE_PROPERTY_NAME, ui->minValueSpin);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, SLIDER_MAX_VALUE_PROPERTY_NAME, ui->maxValueSpin);
}

void SliderPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
	UnregisterDoubleSpinBoxWidget(ui->valueSpin);
    UnregisterDoubleSpinBoxWidget(ui->minValueSpin);
    UnregisterDoubleSpinBoxWidget(ui->maxValueSpin);
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

void SliderPropertyGridWidget::OnSliderValueChanged(int value)
{
	ui->valueSpin->setValue(value);
}