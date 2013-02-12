#include "flagspropertygridwidget.h"
#include "ui_flagspropertygridwidget.h"

static const QString FLAGS_PROPERTY_BLOCK_NAME = "Flags";

FlagsPropertyGridWidget::FlagsPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::FlagsPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(FLAGS_PROPERTY_BLOCK_NAME);
}

FlagsPropertyGridWidget::~FlagsPropertyGridWidget()
{
    delete ui;
}

void FlagsPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);

    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets.
    RegisterCheckBoxWidgetForProperty(propertiesMap, "Selected", ui->selectedCheckBox);
    RegisterCheckBoxWidgetForProperty(propertiesMap, "Visible", ui->visibleCheckBox);
    RegisterCheckBoxWidgetForProperty(propertiesMap, "Enabled", ui->enabledCheckBox);
    RegisterCheckBoxWidgetForProperty(propertiesMap, "Input", ui->inputCheckBox);
    RegisterCheckBoxWidgetForProperty(propertiesMap, "ClipContents", ui->clipContentsCheckbox);
}

void FlagsPropertyGridWidget::Cleanup()
{
    UnregisterCheckBoxWidget(ui->selectedCheckBox);
    UnregisterCheckBoxWidget(ui->visibleCheckBox);
    UnregisterCheckBoxWidget(ui->enabledCheckBox);
    UnregisterCheckBoxWidget(ui->inputCheckBox);
    UnregisterCheckBoxWidget(ui->clipContentsCheckbox);
}
