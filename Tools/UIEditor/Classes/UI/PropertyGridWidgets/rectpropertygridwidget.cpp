#include "rectpropertygridwidget.h"
#include "ui_rectpropertygridwidget.h"

static const QString RECT_PROPERTY_BLOCK_NAME = "Rect";

RectPropertyGridWidget::RectPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::RectPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(RECT_PROPERTY_BLOCK_NAME);
	// Install event filter for all spinboxes on this Widget
	// We should block mouse wheel event for spinboxes which don't have focus
	Q_FOREACH( QSpinBox *spinBoxWidget, findChildren<QSpinBox*>() )
	{
        spinBoxWidget->installEventFilter( this );
        spinBoxWidget->setFocusPolicy( Qt::StrongFocus );
    }
}

RectPropertyGridWidget::~RectPropertyGridWidget()
{
    delete ui;
}

void RectPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
    
    // Initialize the widgets. TODO - understand how to re-use property names!
    RegisterSpinBoxWidgetForProperty(propertiesMap, "RelativeX", ui->relativeXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "RelativeY", ui->relativeYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "AbsoluteX", ui->absoluteXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "AbsoluteY", ui->absoluteYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "SizeX", ui->sizeXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "SizeY", ui->sizeYSpinBox);
    
    RegisterSpinBoxWidgetForProperty(propertiesMap, "PivotX", ui->pivotXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, "PivotY", ui->pivotYSpinBox);

    RegisterSpinBoxWidgetForProperty(propertiesMap, "Angle", ui->angleSpinBox);
}

void RectPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterSpinBoxWidget(ui->relativeXSpinBox);
    UnregisterSpinBoxWidget(ui->relativeYSpinBox);
    UnregisterSpinBoxWidget(ui->absoluteXSpinBox);
    UnregisterSpinBoxWidget(ui->absoluteYSpinBox);
    UnregisterSpinBoxWidget(ui->sizeXSpinBox);
    UnregisterSpinBoxWidget(ui->sizeYSpinBox);
    UnregisterSpinBoxWidget(ui->pivotXSpinBox);
    UnregisterSpinBoxWidget(ui->pivotYSpinBox);
    UnregisterSpinBoxWidget(ui->angleSpinBox);
}

void RectPropertyGridWidget::OnPropertiesChangedFromExternalSource()
{
    // Re-read all the properties related to this grid.
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        UpdateWidgetWithPropertyValue(iter);
    }
}