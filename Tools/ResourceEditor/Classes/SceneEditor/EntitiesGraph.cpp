#include "EntitiesGraph.h"
#include "ControlsFactory.h"
#include "EditorSettings.h"
#include "PropertyControlCreator.h"

EntitiesGraph::EntitiesGraph(GraphBaseDelegate *newDelegate, const Rect &rect)
:   GraphBase(newDelegate, rect),
	workingEntity(0)
{
	CreateGraphPanel(rect);
}

EntitiesGraph::~EntitiesGraph()
{

}

void EntitiesGraph::CreateGraphPanel(const Rect &rect)
{
	GraphBase::CreateGraphPanel(rect);

	Rect graphRect = graphPanel->GetRect();
	graphTree = new UIHierarchy(graphRect);
	ControlsFactory::CusomizeListControl(graphTree);
	ControlsFactory::SetScrollbar(graphTree);
	graphTree->SetCellHeight(ControlsFactory::CELL_HEIGHT);
	graphTree->SetDelegate(this);
	graphTree->SetClipContents(true);
	graphPanel->AddControl(graphTree);
}

int32 EntitiesGraph::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
	return workingScene->entityManager->GetAllEntities().size();
}

void * EntitiesGraph::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
	return workingScene->entityManager->GetAllEntities()[index];
}

void EntitiesGraph::FillCell(UIHierarchyCell *cell, void *node)
{
	Entity *n = (Entity *)node;
	UIStaticText *text =  (UIStaticText *)cell->FindByName("_Text_");
	text->SetText(Format(L"Entity %d", n->GetIndexInFamily()));
}

void EntitiesGraph::SelectNode(BaseObject *node)
{
	if(!node)
	{
		workingEntity = 0;
		RefreshGraph();
	}
}

void EntitiesGraph::SelectHierarchyNode(UIHierarchyNode * node)
{
	Entity * sceneEntity = (Entity*)(node->GetUserNode());
	workingEntity = sceneEntity;
	//workingNode = sceneNode;
	//workingScene->SetSelection(0);
	//workingScene->SetSelection(workingNode);

	UpdatePropertyPanel();
}

void EntitiesGraph::UpdatePropertyPanel()
{
	if(workingEntity)
	{
		RecreatePropertiesPanelForEntity(workingEntity);

		if(propertyControl->GetParent() && propertyControl->GetParent() != propertyPanel)
		{
			propertyControl->GetParent()->RemoveControl(propertyControl);
		}

		if(!propertyControl->GetParent())
		{
			propertyPanel->AddControl(propertyControl);
		}
		RefreshProperties();
	}
	else
	{
		if(propertyControl && propertyControl->GetParent())
		{
			propertyPanel->RemoveControl(propertyControl);
		}
	}
}

void EntitiesGraph::RecreatePropertiesPanelForEntity(Entity * entity)
{
	if(propertyControl && propertyControl->GetParent())
	{
		propertyPanel->RemoveControl(propertyControl);
	}
	SafeRelease(propertyControl);

	propertyControl = PropertyControlCreator::Instance()->CreateControlForEntity(entity, propertyPanelRect);

	SafeRetain(propertyControl);
	propertyControl->SetDelegate(this);
	propertyControl->SetWorkingScene(workingScene);
	propertyControl->ReadFrom(entity);
}


