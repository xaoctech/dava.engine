#include "aggregatorpropertygridwidget.h"
#include "ui_aggregatorpropertygridwidget.h"

static const QString RECT_PROPERTY_BLOCK_NAME = "Aggregator";

AggregatorPropertyGridWidget::AggregatorPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::AggregatorPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);
}

AggregatorPropertyGridWidget::~AggregatorPropertyGridWidget()
{
    delete ui;
}

void AggregatorPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets.
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->platformNameLineEdit, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "Height", ui->screenHeightSpinBox, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "Width", ui->screenWidthSpinBox, true);
}

void AggregatorPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterLineEditWidget(ui->platformNameLineEdit);
    UnregisterSpinBoxWidget(ui->screenHeightSpinBox);
    UnregisterSpinBoxWidget(ui->screenWidthSpinBox);
}