#include "controlpropertygridwidget.h"
#include "ui_controlpropertygridwidget.h"

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

#include "PropertyNames.h"
#include "BaseMetadata.h"

#include "BaseCommand.h"
#include "ChangePropertyCommand.h"

#include "UIAggregatorMetadata.h"

using namespace DAVA;
using namespace PropertyNames;

static const QString DEFAULT_CONTROL_PROPERTY_BLOCK_NAME = "UI Control";
static const QString CUSTOM_CONTROL_PROPERTY_BLOCK_NAME = "Custom Control";

ControlPropertyGridWidget::ControlPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ControlPropertyGridWidget),
	currentWidgetState(STATE_DEFAULT_CONTROL)
{
    ui->setupUi(this);
	
	this->ui->frameCustomControlData->setHidden(true);
	this->ui->frameMorphToCustomControl->setHidden(true);

	SetWidgetState(STATE_DEFAULT_CONTROL, true);
	ConnectToSignals();
}

ControlPropertyGridWidget::~ControlPropertyGridWidget()
{
	DisconnectFromSignals();
    delete ui;
}

void ControlPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets. TODO - understand how to re-use property names!
    RegisterLineEditWidgetForProperty(propertiesMap, "UIControlClassName", ui->classNameLineEdit);
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->objectNameLineEdit, true);
    RegisterLineEditWidgetForProperty(propertiesMap, "Tag", ui->tagLineEdit);
	RegisterLineEditWidgetForProperty(propertiesMap, CUSTOM_CONTROL_NAME, ui->customControlLineEdit);
	
	UpdatePropertiesForSubcontrol();
}

void ControlPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterLineEditWidget(ui->objectNameLineEdit);
    UnregisterLineEditWidget(ui->tagLineEdit);
	UnregisterLineEditWidget(ui->customControlLineEdit);
}

// Change the state of the widget.
void ControlPropertyGridWidget::SetWidgetState(eWidgetState newState, bool forceUpdate)
{
	if (!forceUpdate && newState == currentWidgetState)
	{
		return;
	}

	static const int WIDGET_HEIGHT_DEFAULT = 160;
	static const int WIDGET_HEIGHT_CUSTOM = 200;
	static const int GROUP_YPOS = 120;

	switch (newState)
	{
		case STATE_DEFAULT_CONTROL:
		{
			this->resize(this->width(), WIDGET_HEIGHT_DEFAULT);
			this->setMaximumHeight(WIDGET_HEIGHT_DEFAULT);
			this->setMinimumHeight(WIDGET_HEIGHT_DEFAULT);

			this->ui->groupWidgetFrame->resize(this->ui->groupWidgetFrame->width(),
											   WIDGET_HEIGHT_DEFAULT - 1);
			this->ui->frameMorphToCustomControl->move(0, GROUP_YPOS);
			
			this->ui->frameCustomControlData->setHidden(true);
			this->ui->frameMorphToCustomControl->setHidden(false);
			
		    SetPropertyBlockName(DEFAULT_CONTROL_PROPERTY_BLOCK_NAME);
			
			break;
		}

		case STATE_CUSTOM_CONTROL:
		{
			this->resize(this->width(), WIDGET_HEIGHT_CUSTOM);
			this->setMaximumHeight(WIDGET_HEIGHT_CUSTOM);
			this->setMinimumHeight(WIDGET_HEIGHT_CUSTOM);

			this->ui->groupWidgetFrame->resize(this->ui->groupWidgetFrame->width(),
											   WIDGET_HEIGHT_CUSTOM - 1);
			this->ui->frameCustomControlData->move(0, GROUP_YPOS);
			
			this->ui->frameCustomControlData->setHidden(false);
			this->ui->frameMorphToCustomControl->setHidden(true);
			
			SetPropertyBlockName(CUSTOM_CONTROL_PROPERTY_BLOCK_NAME);
			
			break;
		}

		default:
		{
			break;
		}
	}

	this->currentWidgetState = newState;
}

void ControlPropertyGridWidget::ConnectToSignals()
{
	connect(this->ui->btnMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnMorphToCustomControlClicked()));
	connect(this->ui->btnResetMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnResetMorphToCustomControlClicked()));
}

void ControlPropertyGridWidget::DisconnectFromSignals()
{
	disconnect(this->ui->btnMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnMorphToCustomControlClicked()));
	disconnect(this->ui->btnResetMorphToCustomControl, SIGNAL(clicked()),
			this, SLOT(OnResetMorphToCustomControlClicked()));
}

void ControlPropertyGridWidget::OnMorphToCustomControlClicked()
{
	// Just set the widget state to Custom Control. If the name of the Custom Control
	// will not be explicitely entered by the user, the state will be reset.
	SetWidgetState(STATE_CUSTOM_CONTROL);
}

void ControlPropertyGridWidget::OnResetMorphToCustomControlClicked()
{
	// Reset the state of the Control by changing the property.
	PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(ui->customControlLineEdit);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnResetMorphToCustomControlClicked - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }

	BaseCommand* command = new ChangePropertyCommand<QString>(activeMetadata, iter->second, QString());
    CommandsController::Instance()->ExecuteCommand(command);
	SafeRelease(command);
	
	// The state will be changed later on - in the UpdateLineEditWidgetWithPropertyValue().
}

void ControlPropertyGridWidget::UpdateLineEditWidgetWithPropertyValue(QLineEdit* lineEditWidget,
                                                                   const QMetaProperty& curProperty)
{
	BasePropertyGridWidget::UpdateLineEditWidgetWithPropertyValue(lineEditWidget, curProperty);
	
	if (lineEditWidget != this->ui->customControlLineEdit)
	{
		return;
	}
	
	// Determine the new widget state depending on the value.
	eWidgetState widgetState = this->ui->customControlLineEdit->text().isEmpty() ?
		STATE_DEFAULT_CONTROL : STATE_CUSTOM_CONTROL;
	SetWidgetState(widgetState);
}

void ControlPropertyGridWidget::UpdatePropertiesForSubcontrol()
{
	if (!activeMetadata || !activeMetadata->GetParamsCount())
	{
		return;
	}

	bool isSubcontrol = ActiveControlIsSubcontrol();

	// Several properties can't be changed for subcontrols.
	this->ui->customControlLineEdit->setReadOnly(isSubcontrol);
	this->ui->objectNameLineEdit->setReadOnly(isSubcontrol);
	this->ui->btnMorphToCustomControl->setEnabled(!isSubcontrol);
	
	// Hide morph button for UI Aggregator
	UIAggregatorMetadata *UIAggregatorMeta = dynamic_cast<UIAggregatorMetadata*>(activeMetadata);
	this->ui->btnMorphToCustomControl->setHidden(UIAggregatorMeta ? true : false);
}
