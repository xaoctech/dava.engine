#include "spinnerpropertygridwidget.h"
#include "ui_spinnerpropertygridwidget.h"
#include "PropertyNames.h"

static const QString SPINNER_PROPERTY_BLOCK_NAME = "Spinner";

SpinnerPropertyGridWidget::SpinnerPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::SpinnerPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(SPINNER_PROPERTY_BLOCK_NAME);
}

SpinnerPropertyGridWidget::~SpinnerPropertyGridWidget()
{
    delete ui;
}

void SpinnerPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
	BasePropertyGridWidget::Initialize(activeMetadata);

    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	
	RegisterLineEditWidgetForProperty(propertiesMap, DAVA::PropertyNames::UISPINNER_PREV_BUTTON_TEXT, ui->lineEditPrevButtonText);
	RegisterLineEditWidgetForProperty(propertiesMap, DAVA::PropertyNames::UISPINNER_NEXT_BUTTON_TEXT, ui->lineEditNextButtonText);
}

void SpinnerPropertyGridWidget::Cleanup()
{
	UnregisterLineEditWidget(ui->lineEditPrevButtonText);
	UnregisterLineEditWidget(ui->lineEditNextButtonText);

	BasePropertyGridWidget::Cleanup();
}
