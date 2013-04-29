#include "scrollviewpropertygridwidget.h"
#include "ui_scrollviewpropertygridwidget.h"

#include "PropertyNames.h"
#include "BackgroundGridWidgetHelper.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"
#include "WidgetSignalsBlocker.h"
#include "ResourcesManageHelper.h"
#include "ResourcePacker.h"

#include <QFileDialog>

using namespace PropertyNames;

static const QString SCROLL_VIEW_PROPERTY_BLOCK_NAME = "Scroll area options";

ScrollViewPropertyGridWidget::ScrollViewPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ScrollViewPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(SCROLL_VIEW_PROPERTY_BLOCK_NAME);
    ConnectToSignals();

    BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

ScrollViewPropertyGridWidget::~ScrollViewPropertyGridWidget()
{
    delete ui;
}

void ScrollViewPropertyGridWidget::ConnectToSignals()
{
    // Create connection between slider and double spinbox
    connect(ui->scrollHSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
    connect(ui->scrollVSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
}

void ScrollViewPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);

    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, HORIZONTAL_SCROLL_POSITION, ui->scrollHPositionSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, VERTICAL_SCROLL_POSITION, ui->scrollVPositionSpinBox);
}

void ScrollViewPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    UnregisterDoubleSpinBoxWidget(ui->scrollHPositionSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->scrollVPositionSpinBox);
}

void ScrollViewPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox *doubleSpinBoxWidget,
                                                        const PROPERTYGRIDWIDGETSITER &iter, const double value)
{
    if (doubleSpinBoxWidget == NULL)
    {
        Logger::Error("ScrollViewPropertyGridWidget::ProcessDoubleSpinBoxValueChanged: senderWidget is NULL!");
        return;
    }

    // Try to process this control-specific widgets.
    if (doubleSpinBoxWidget == ui->scrollHPositionSpinBox || doubleSpinBoxWidget == ui->scrollVPositionSpinBox)
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

void ScrollViewPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *doubleSpinBoxWidget,
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
    if (doubleSpinBoxWidget == ui->scrollHPositionSpinBox)
    {
         //Upate value of slider
        ui->scrollHSlider->setValue((int)propertyValue);
        return;
    }
    else if (doubleSpinBoxWidget == ui->scrollVPositionSpinBox)
    {
        //Upate value of slider
        ui->scrollVSlider->setValue((int)propertyValue);
        return;
    }

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(doubleSpinBoxWidget, curProperty);
}

void ScrollViewPropertyGridWidget::OnSliderValueChanged(int value)
{
    QWidget* senderWidget = (QWidget*) QObject::sender();

    if (senderWidget == ui->scrollHSlider)
    {
        ui->scrollHPositionSpinBox->setValue(value);
    }
    else if (senderWidget == ui->scrollVSlider)
    {
        ui->scrollVPositionSpinBox->setValue(value);
    }
}
