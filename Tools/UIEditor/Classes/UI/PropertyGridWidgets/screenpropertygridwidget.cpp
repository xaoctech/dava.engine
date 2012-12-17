#include "screenpropertygridwidget.h"
#include "ui_screenpropertygridwidget.h"

static const QString RECT_PROPERTY_BLOCK_NAME = "Screen";

ScreenPropertyGridWidget::ScreenPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ScreenPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);
}

ScreenPropertyGridWidget::~ScreenPropertyGridWidget()
{
    delete ui;
}

void ScreenPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets.
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->screenNameLineEdit, true);
}

void ScreenPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
    UnregisterLineEditWidget(ui->screenNameLineEdit);
}