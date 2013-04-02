#include "propertygridcontainerwidget.h"
#include "ui_propertygridcontainerwidget.h"

#include "MetadataFactory.h"

PropertyGridContainerWidget::PropertyGridContainerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PropertyGridContainerWidget),
    activeMetadata(NULL)
{
    ui->setupUi(this);
	ui->topArea->hide();

    widgetsFactory.InitializeWidgetParents(this);
    
    ConnectToSignals();
}

PropertyGridContainerWidget::~PropertyGridContainerWidget()
{
    delete ui;
}

void PropertyGridContainerWidget::ConnectToSignals()
{
    connect(PropertiesGridController::Instance(), SIGNAL(PropertiesGridUpdated()),
            this, SLOT(OnPropertiesGridUpdated()));
}

void PropertyGridContainerWidget::OnPropertiesGridUpdated()
{
    if (PropertiesGridController::Instance()->GetActiveTreeNodesList().empty() == false)
    {
        // List of UI Controls is selected.
        BuildPropertiesGridList();
        return;
    }
    
    // Build the Properties Grid based on the active control.
    const HierarchyTreeNode* activeTreeNode = PropertiesGridController::Instance()->GetActiveTreeNode();
    if (activeTreeNode == NULL)
    {
        // Nothing is selected - cleanup the properties grid.
        CleanupPropertiesGrid();
        return;
    }

    UIControl* activeControl = NULL;
    const HierarchyTreeControlNode* activeTreeControlNode = dynamic_cast<const HierarchyTreeControlNode*>(activeTreeNode);
    if (activeTreeControlNode)
    {
        activeControl = activeTreeControlNode->GetUIObject();
    }

    BaseMetadata* metaData = GetActiveMetadata(activeTreeNode);    
    BuildPropertiesGrid(activeControl, metaData, activeTreeNode->GetId());
}

void PropertyGridContainerWidget::CleanupPropertiesGrid()
{
    // Cleanup the properties grid.
    for (PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSITER iter = activeWidgetsList.begin();
         iter != activeWidgetsList.end(); iter ++)
    {
        BasePropertyGridWidget* widget = (*iter);
        widget->Cleanup();

		if (!dynamic_cast<StatePropertyGridWidget*>(*iter))
		{
	        ui->scrollAreaLayout->removeWidget(widget);
		}
		else
		{
			ui->topAreaLayout->removeWidget(widget);
		}

        widget->hide();
    }

	ui->topArea->hide();

    // Also release the Active Metadata, if any.
    CleanupActiveMetadata();
}

void PropertyGridContainerWidget::BuildPropertiesGridList()
{
    // Need to understand which Metadata we need. Since multiple controls of different
    // type might be selected, we are looking for the most common Metadata.
    const HierarchyTreeController::SELECTEDCONTROLNODES activeNodes = PropertiesGridController::Instance()->GetActiveTreeNodesList();
    BaseMetadata* metaData = GetActiveMetadata(activeNodes);

    METADATAPARAMSVECT params;
    for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = activeNodes.begin();
         iter != activeNodes.end(); iter ++)
    {
        const HierarchyTreeControlNode* controlNode = *iter;
        BaseMetadataParams controlNodeParam(controlNode->GetId(), controlNode->GetUIObject());
        params.push_back(controlNodeParam);
    }

    BuildPropertiesGrid(metaData, params);
}

void PropertyGridContainerWidget::BuildPropertiesGrid(UIControl* activeControl,
                                                      BaseMetadata* metaData,
                                                      HierarchyTreeNode::HIERARCHYTREENODEID activeNodeID)
{
    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(activeNodeID, activeControl));
    BuildPropertiesGrid(metaData, params);
}

void PropertyGridContainerWidget::BuildPropertiesGrid(BaseMetadata* metaData,
                                                      const METADATAPARAMSVECT& params)
{
    CleanupPropertiesGrid();

    // Metadata is state-aware.
	Vector<UIControl::eControlState> activeStates = PropertiesGridController::Instance()->GetActiveUIControlStates();
    metaData->SetUIControlStates(activeStates);

    this->activeWidgetsList = widgetsFactory.GetWidgets(metaData);
    for (PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSITER iter = activeWidgetsList.begin();
         iter != activeWidgetsList.end(); iter ++)
    {
		BasePropertyGridWidget* widget = (*iter);

		if (!dynamic_cast<StatePropertyGridWidget*>(*iter))
		{
			ui->scrollAreaLayout->addWidget(widget);
		}
		else
		{
			ui->topArea->show();
			ui->topAreaLayout->addWidget(widget);
		}
		widget->show();

		// Eaach widget should initialize itself by using Metadata (for property names) and
		// UI Control (for actual values).
		metaData->SetupParams(params);
		widget->Initialize(metaData);
    }
    
    // The Active Metatata is known.
    this->activeMetadata = metaData;
}

// Work with Active Metadata.
BaseMetadata* PropertyGridContainerWidget::GetActiveMetadata(const HierarchyTreeNode* activeTreeNode)
{
    if (activeTreeNode)
    {
        return MetadataFactory::Instance()->GetMetadataForTreeNode(activeTreeNode);
    }
    
    return NULL;
}

// Work with Active Metadata.
BaseMetadata* PropertyGridContainerWidget::GetActiveMetadata(const HierarchyTreeController::SELECTEDCONTROLNODES activeNodes)
{
    return MetadataFactory::Instance()->GetMetadataForTreeNodesList(activeNodes);
}

void PropertyGridContainerWidget::CleanupActiveMetadata()
{
    SAFE_DELETE(this->activeMetadata);
}
