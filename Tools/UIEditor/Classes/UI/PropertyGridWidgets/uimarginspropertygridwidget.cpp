#include "uimarginspropertygridwidget.h"
#include "ui_uimarginspropertygridwidget.h"

#include "PropertyNames.h"
#include "WidgetSignalsBlocker.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"

static const QString MARGINS_PROPERTY_BLOCK_NAME = "Text Margins";

UIMarginsPropertyGridWidget::UIMarginsPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::UIMarginsPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(MARGINS_PROPERTY_BLOCK_NAME);
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);

}

UIMarginsPropertyGridWidget::~UIMarginsPropertyGridWidget()
{
    delete ui;
}


void UIMarginsPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);

    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::UIMARGIN_LEFT_PROPERTY_NAME, ui->leftDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::UIMARGIN_TOP_PROPERTY_NAME, ui->topDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::UIMARGIN_RIGHT_PROPERTY_NAME, ui->rightDoubleSpinBox);
    RegisterDoubleSpinBoxWidgetForProperty(propertiesMap, PropertyNames::UIMARGIN_BOTTOM_PROPERTY_NAME, ui->bottomDoubleSpinBox);

    connect(ui->resetMarginsButton, SIGNAL(clicked()), this, SLOT(OnResetUIMarginsClicked()));
    UpdateUI();
}

void UIMarginsPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    disconnect(ui->resetMarginsButton, SIGNAL(clicked()), this, SLOT(OnResetUIMarginsClicked()));
  
    UnregisterDoubleSpinBoxWidget(ui->leftDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->topDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->rightDoubleSpinBox);
    UnregisterDoubleSpinBoxWidget(ui->bottomDoubleSpinBox);
}

void UIMarginsPropertyGridWidget::UpdateDoubleSpinBoxWidgetWithPropertyValue(QDoubleSpinBox *spinBoxWidget,
                                                                             const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }
    
	double propertyValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata,
                                                                          curProperty.name());
    WidgetSignalsBlocker blocker(spinBoxWidget);
    spinBoxWidget->setValue(propertyValue);
    UpdateUI();
}

void UIMarginsPropertyGridWidget::ProcessDoubleSpinBoxValueChanged(QDoubleSpinBox* /*doubleSpinBox*/,
                                                                   const PROPERTYGRIDWIDGETSITER &iter,
                                                                   const double value)
{
    double curValue = PropertiesHelper::GetAllPropertyValues<double>(this->activeMetadata, iter->second.getProperty().name());
    if (curValue == value)
    {
        return;
    }

    BaseCommand* command = new ChangeDoublePropertyCommand(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void UIMarginsPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    UpdateUI();
}

void UIMarginsPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);
    UpdateUI();
}

void UIMarginsPropertyGridWidget::UpdateUI()
{
    if (!activeMetadata)
    {
        return;
    }

    UpdateMaxMarginValues();
    UpdateResetMarginsButtonVisible();
}

void UIMarginsPropertyGridWidget::UpdateMaxMarginValues()
{
    float sizeX = PropertiesHelper::GetAllPropertyValues<float>(activeMetadata, PropertyNames::SIZE_X);
    ui->rightDoubleSpinBox->setMaximum(sizeX - ui->leftDoubleSpinBox->value());
    ui->leftDoubleSpinBox->setMaximum(sizeX - ui->rightDoubleSpinBox->value());

    float sizeY = PropertiesHelper::GetAllPropertyValues<float>(activeMetadata, PropertyNames::SIZE_Y);
    ui->bottomDoubleSpinBox->setMaximum(sizeY - ui->topDoubleSpinBox->value());
    ui->topDoubleSpinBox->setMaximum(sizeY - ui->bottomDoubleSpinBox->value());
}

void UIMarginsPropertyGridWidget::UpdateResetMarginsButtonVisible()
{
    bool needShowButton = !(FLOAT_EQUAL(ui->leftDoubleSpinBox->value(), 0.0f) &&
                            FLOAT_EQUAL(ui->topDoubleSpinBox->value(), 0.0f) &&
                            FLOAT_EQUAL(ui->rightDoubleSpinBox->value(), 0.0f) &&
                            FLOAT_EQUAL(ui->bottomDoubleSpinBox->value(), 0.0f));
    ui->resetMarginsButton->setVisible(needShowButton);
}

void UIMarginsPropertyGridWidget::OnResetUIMarginsClicked()
{
    if (!activeMetadata)
    {
        return;
    }
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	PROPERTIESMAPITER uiMarginsIter = propertiesMap.find(PropertyNames::UIMARGINS_PROPERTY_NAME);
	DVASSERT(uiMarginsIter != propertiesMap.end());

    BaseCommand* command = new ChangePropertyCommand<QRectF>(activeMetadata, PropertyGridWidgetData(uiMarginsIter->second, false, false), QRectF());
	CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);

	CommandsController::Instance()->EmitUpdatePropertyValues();
}
