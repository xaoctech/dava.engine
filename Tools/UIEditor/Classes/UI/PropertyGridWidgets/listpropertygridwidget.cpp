#include "listpropertygridwidget.h"
#include "ui_listpropertygridwidget.h"

#include "UIListMetadata.h"

#include "WidgetSignalsBlocker.h"
#include "ChangePropertyCommand.h"
#include "CommandsController.h"

static const QString LIST_PROPERTY_BLOCK_NAME = "List";

ListPropertyGridWidget::ListPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ListPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(LIST_PROPERTY_BLOCK_NAME);
}

ListPropertyGridWidget::~ListPropertyGridWidget()
{
    delete ui;
}

void ListPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);	
	FillCombobox();
	
	// Build the properties map to make the properties search faster.
	PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();
	
	RegisterComboBoxWidgetForProperty(propertiesMap, "AggregatorID", ui->aggregatorsComboBox, false, true);
}

void ListPropertyGridWidget::Cleanup()
{	
	UnregisterComboBoxWidget(ui->aggregatorsComboBox);	
}

void ListPropertyGridWidget::FillCombobox()
{
    WidgetSignalsBlocker aggregatorsBlocker(ui->aggregatorsComboBox);
	// Reset lists
	ui->aggregatorsComboBox->clear();
	nodeIDList.clear();

	// Add empty aggregator and set its id as "0"
	ui->aggregatorsComboBox->addItem(QString("none"));
	nodeIDList.push_back(0);
	
	// Put into combobox available aggregators names
	const HierarchyTreePlatformNode* platform = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platform)
	{
		const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = platform->GetChildNodes();
		for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
			 iter != items.end();
			 ++iter)
		{
			HierarchyTreeNode* node = (*iter);
			// Get aggregator node id and put it into internal list
			HierarchyTreeAggregatorNode* aggregatorNode = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
			if (aggregatorNode)
			{
				ui->aggregatorsComboBox->addItem(aggregatorNode->GetName());
				nodeIDList.push_back(aggregatorNode->GetId());
			}
		}
	}
}

void ListPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                         const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("BackGroundPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();
    
    if (senderWidget == ui->aggregatorsComboBox)
    {
		return CustomProcessComboboxValueChanged(iter, nodeIDList.at(selectedIndex));
    }

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void ListPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
{
	// Don't update the property if the text wasn't actually changed.
    int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}

	BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void ListPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
    if (comboBoxWidget == ui->aggregatorsComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
		int index = nodeIDList.indexOf(propertyValue);
		
        return SetComboboxSelectedItem(comboBoxWidget, ui->aggregatorsComboBox->itemText(index));
    }

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}