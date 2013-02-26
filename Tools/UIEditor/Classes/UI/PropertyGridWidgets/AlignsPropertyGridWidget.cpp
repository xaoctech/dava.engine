#include "alignspropertygridwidget.h"
#include "ui_alignspropertygridwidget.h"

#include "PropertyNames.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"
#include "WidgetSignalsBlocker.h"
#include "ResourcesManageHelper.h"
#include "PropertiesGridController.h"

using namespace PropertyNames;

static const QString ALIGN_PROPERTY_BLOCK_NAME = "Align options";

AlignsPropertyGridWidget::AlignsPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::AlignsPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(ALIGN_PROPERTY_BLOCK_NAME);
	
	// Create align checkbox-spinbox map
	// This map should help to manage checkbox/spinbox releations
	alignWidgetsMap = ALIGNWIDGETSMAP();
	alignWidgetsMap.insert(std::make_pair(ui->leftAlignCheckBox, ui->leftAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->hcenterAlignCheckBox, ui->hcenterAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->rightAlignCheckBox, ui->rigthAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->topAlignCheckBox, ui->topAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->vcenterAlignCheckBox, ui->vcenterAlignSpinBox));
	alignWidgetsMap.insert(std::make_pair(ui->bottomAlignCheckBox, ui->bottomAlignSpinBox));

	// This event filter will prevent mouse wheel event when hovered control is not yet selected
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

AlignsPropertyGridWidget::~AlignsPropertyGridWidget()
{
    delete ui;
}

void AlignsPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	
	RegisterCheckBoxWidgetForProperty(propertiesMap, LEFT_ALIGN_ENABLED, ui->leftAlignCheckBox);
	RegisterCheckBoxWidgetForProperty(propertiesMap, HCENTER_ALIGN_ENABLED, ui->hcenterAlignCheckBox);
	RegisterCheckBoxWidgetForProperty(propertiesMap, RIGHT_ALIGN_ENABLED, ui->rightAlignCheckBox);
	
	RegisterCheckBoxWidgetForProperty(propertiesMap, TOP_ALIGN_ENABLED, ui->topAlignCheckBox);
	RegisterCheckBoxWidgetForProperty(propertiesMap, VCENTER_ALIGN_ENABLED, ui->vcenterAlignCheckBox);
	RegisterCheckBoxWidgetForProperty(propertiesMap, BOTTOM_ALIGN_ENABLED, ui->bottomAlignCheckBox);

    // Register the widgets.
    RegisterSpinBoxWidgetForProperty(propertiesMap, LEFT_ALIGN, ui->leftAlignSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, HCENTER_ALIGN, ui->hcenterAlignSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, RIGHT_ALIGN, ui->rigthAlignSpinBox);
	
    RegisterSpinBoxWidgetForProperty(propertiesMap, TOP_ALIGN, ui->topAlignSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, VCENTER_ALIGN, ui->vcenterAlignSpinBox);
	RegisterSpinBoxWidgetForProperty(propertiesMap, BOTTOM_ALIGN, ui->bottomAlignSpinBox);
}

void AlignsPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();
	
	UnregisterCheckBoxWidget(ui->leftAlignCheckBox);
	UnregisterCheckBoxWidget(ui->hcenterAlignCheckBox);
	UnregisterCheckBoxWidget(ui->rightAlignCheckBox);
	UnregisterCheckBoxWidget(ui->topAlignCheckBox);
	UnregisterCheckBoxWidget(ui->vcenterAlignCheckBox);
	UnregisterCheckBoxWidget(ui->bottomAlignCheckBox);	
	
	UnregisterSpinBoxWidget(ui->leftAlignSpinBox);
	UnregisterSpinBoxWidget(ui->hcenterAlignSpinBox);
	UnregisterSpinBoxWidget(ui->rigthAlignSpinBox);
	UnregisterSpinBoxWidget(ui->topAlignSpinBox);
	UnregisterSpinBoxWidget(ui->vcenterAlignSpinBox);
	UnregisterSpinBoxWidget(ui->bottomAlignSpinBox);
}

void AlignsPropertyGridWidget::OnPropertiesChangedFromExternalSource()
{
    // Re-read all the properties related to this grid.
    for (PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.begin();
         iter != this->propertyGridWidgetsMap.end(); iter ++)
    {
        UpdateWidgetWithPropertyValue(iter);
    }
}

void AlignsPropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(QCheckBox* checkBoxWidget,
                                                                   const QMetaProperty& curProperty)
{
	BasePropertyGridWidget::UpdateCheckBoxWidgetWithPropertyValue(checkBoxWidget, curProperty);

	UpdateCheckBoxSates();
	UpdateSpinBoxState(checkBoxWidget);
}

void AlignsPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
	// Each time aligns properties are changes we should notify all listeners
	// (first of all in rect widget) to force recalculation of related properties
	// In other words if we changed align option we should force change of related
	// property in Rect Property Grid widget
	CommandsController::Instance()->EmitUpdatePropertyValues();
}

void AlignsPropertyGridWidget::UpdateCheckBoxSates()
{
	// Horizontal align
	// Only two horizontal align properties can be set per time
	// We have to disable third checkbox if two other checkboxes already selected
	bool disableCheckBox = ui->hcenterAlignCheckBox->isChecked() && ui->rightAlignCheckBox->isChecked();
    ui->leftAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->leftAlignCheckBox->isChecked() && ui->rightAlignCheckBox->isChecked();
    ui->hcenterAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->leftAlignCheckBox->isChecked() && ui->hcenterAlignCheckBox->isChecked();
    ui->rightAlignCheckBox->setDisabled(disableCheckBox);
	
	// Vertical align
    disableCheckBox = ui->vcenterAlignCheckBox->isChecked() && ui->bottomAlignCheckBox->isChecked();
    ui->topAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->topAlignCheckBox->isChecked() && ui->bottomAlignCheckBox->isChecked();
    ui->vcenterAlignCheckBox->setDisabled(disableCheckBox);
	
    disableCheckBox = ui->topAlignCheckBox->isChecked() && ui->vcenterAlignCheckBox->isChecked();
    ui->bottomAlignCheckBox->setDisabled(disableCheckBox);
}

void AlignsPropertyGridWidget::UpdateSpinBoxState(QCheckBox *buddyWidget)
{
	WidgetSignalsBlocker blocker(buddyWidget);
	// Get current state of checkbox
	bool state = buddyWidget->isChecked();
	
	// Update appropriate spinbox state
	ALIGNWIDGETSMAPITER iter = this->alignWidgetsMap.find(buddyWidget);
    if (iter != this->alignWidgetsMap.end())
    {
     	QSpinBox *spinBox = iter->second;
		// Change spinbox state according to checkbox state
		spinBox->setEnabled(state);
		// If spinbox was switched to disabled - we should reset its value
		if (!state)
		{
			spinBox->setValue(0);
		}
    }
}
