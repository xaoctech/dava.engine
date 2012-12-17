#include "platformpropertygridwidget.h"
#include "ui_platformpropertygridwidget.h"

static const QString RECT_PROPERTY_BLOCK_NAME = "Platform";

PlatformPropertyGridWidget::PlatformPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::PlatformPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);
}

PlatformPropertyGridWidget::~PlatformPropertyGridWidget()
{
    delete ui;
}

void PlatformPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets.
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->platformNameLineEdit, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "Height", ui->screenHeightSpinBox, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "Width", ui->screenWidthSpinBox, true);
}

void PlatformPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterLineEditWidget(ui->platformNameLineEdit);
    UnregisterSpinBoxWidget(ui->screenHeightSpinBox);
    UnregisterSpinBoxWidget(ui->screenWidthSpinBox);
}