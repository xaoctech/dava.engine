//
//  HierarchyTreeController.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreeController.h"
#include "ScreenWrapper.h"
#include "DefaultScreen.h"

#include "BaseMetadata.h"
#include "MetadataFactory.h"

HierarchyTreeController::HierarchyTreeController(QObject* parent) :
	QObject(parent)
{
	Clear();
}

HierarchyTreeController::~HierarchyTreeController()
{
    
}

bool HierarchyTreeController::Load(const QString& projectPath)
{
	CloseProject();
	
	bool res = hierarchyTree.Load(projectPath);

	emit HierarchyTreeUpdated();
	emit ProjectLoaded();
	
	return res;
}

bool HierarchyTreeController::Save(const QString& projectPath)
{
	bool res = hierarchyTree.Save(projectPath);
	emit ProjectSaved();
	return res;
}

void HierarchyTreeController::UpdateSelection(const HierarchyTreePlatformNode* activePlatform,
											  const HierarchyTreeScreenNode* activeScreen)
{
	if (this->activePlatform != activePlatform)
	{
		ResetSelectedControl();
		this->activePlatform = (HierarchyTreePlatformNode*)activePlatform;
		if (this->activePlatform)
			this->activePlatform->ActivatePlatform();

        // The platform is changed - update the Localization System.
        UpdateLocalization(false);

		emit SelectedPlatformChanged(this->activePlatform);
	}
	if (this->activeScreen != activeScreen)
	{
		ResetSelectedControl();
		this->activeScreen = (HierarchyTreeScreenNode*)activeScreen;
		emit SelectedScreenChanged(this->activeScreen);
	}
}

void HierarchyTreeController::UpdateSelection(const HierarchyTreeNode* activeItem)
{
	emit SelectedTreeItemChanged(activeItem);
}

void HierarchyTreeController::ChangeItemSelection(HierarchyTreeControlNode* control)
{
	if (IsControlSelected(control))
		UnselectControl(control);
	else
		SelectControl(control);
}

void HierarchyTreeController::SelectControl(HierarchyTreeControlNode* control)
{
	if (activeControlNodes.find(control) != activeControlNodes.end())
		return;
	
	//add selection
	activeControlNodes.insert(control);
	control->GetUIObject()->SetDebugDraw(true);
	emit AddSelectedControl(control);
	emit SelectedControlNodesChanged(activeControlNodes);
	UpdateSelection(control);
}

void HierarchyTreeController::UnselectControl(HierarchyTreeControlNode* control)
{
	if (activeControlNodes.find(control) == activeControlNodes.end())
		return;
	
	//remove selection
	//activeControlNodes.erase(iter);
	activeControlNodes.erase(control);
	control->GetUIObject()->SetDebugDraw(false);
	emit RemoveSelectedControl(control);
	emit SelectedControlNodesChanged(activeControlNodes);
}

bool HierarchyTreeController::IsControlSelected(HierarchyTreeControlNode* control) const
{
	return (activeControlNodes.find(control) != activeControlNodes.end());
}

void HierarchyTreeController::ResetSelectedControl()
{
	for (SELECTEDCONTROLNODES::iterator iter = activeControlNodes.begin();
		 iter != activeControlNodes.end();
		 ++iter)
	{
		HierarchyTreeControlNode* activeControl = (*iter);
		activeControl->GetUIObject()->SetDebugDraw(false);
		emit RemoveSelectedControl(activeControl);
	}

	activeControlNodes.clear();
	emit SelectedControlNodesChanged(activeControlNodes);
}

void HierarchyTreeController::Clear()
{
	activePlatform = NULL;
    activeScreen = NULL;
	
	ResetSelectedControl();
}

void HierarchyTreeController::CreateNewControl(const QString& strType, const QPoint& position)
{
	if (!activeScreen)
		return;
		
    // Create the control itself.
	String type = strType.toStdString();
	BaseObject* object = ObjectFactory::Instance()->New(type);
	UIControl* control = dynamic_cast<UIControl*>(object);
	if (!control)
	{
		SafeRelease(object);
		return;
	}
	
	HierarchyTreeNode* parentNode = activeScreen;
	Vector2 parentDelta(0, 0);
	if (activeControlNodes.size() == 1)
	{
		HierarchyTreeControlNode* parentControlNode = (*activeControlNodes.begin());
		parentNode = parentControlNode;
		//parentDelta = parentControlNode->GetUIObject()->GetPosition();
		parentDelta = parentControlNode->GetParentDelta();
	}

    // Add the tree node - we need it before initializing control.
    String newName = activeScreen->GetNewControlName(type);
	HierarchyTreeControlNode* controlNode = new HierarchyTreeControlNode(parentNode, control,
                                                                         QString::fromStdString(newName));
	parentNode->AddTreeNode(controlNode);

    // Initialize a control through its metadata.
    BaseMetadata* newControlMetadata = MetadataFactory::Instance()->GetMetadataForUIControl(control);

    METADATAPARAMSVECT params;
    params.push_back(BaseMetadataParams(controlNode->GetId(), control));
    newControlMetadata->SetupParams(params);

    // Pre-calculate the values needed for initialization.
    Vector2 point = Vector2(position.x(), position.y());
	DefaultScreen* screen = ScreenWrapper::Instance()->GetActiveScreen();
	if (screen)
		point = screen->LocalToInternal(point);
	point -= parentDelta;

    // Ready to do initialization!
    newControlMetadata->InitializeControl(newName, point);

    SAFE_DELETE(newControlMetadata);
    
	SafeRelease(object);

	emit HierarchyTreeUpdated();
	ResetSelectedControl();
	SelectControl(controlNode);
}

HierarchyTreePlatformNode* HierarchyTreeController::GetActivePlatform() const
{
	return activePlatform;
}

HierarchyTreeScreenNode* HierarchyTreeController::GetActiveScreen() const
{
	return activeScreen;
}

const HierarchyTreeController::SELECTEDCONTROLNODES& HierarchyTreeController::GetActiveControlNodes() const
{
	return activeControlNodes;
}

bool HierarchyTreeController::IsNodeActive(const HierarchyTreeControlNode* control) const
{
	return (activeControlNodes.find((HierarchyTreeControlNode* )control) != activeControlNodes.end());
}

void HierarchyTreeController::EmitHierarchyTreeUpdated()
{
    emit HierarchyTreeUpdated();
}

bool HierarchyTreeController::NewProject(const QString& projectPath)
{
	hierarchyTree.CreateProject();
	emit ProjectCreated();
	return Save(projectPath);
}

void HierarchyTreeController::AddPlatform(const QString& name, const Vector2& size)
{
	hierarchyTree.AddPlatform(name, size);
	EmitHierarchyTreeUpdated();
}

void HierarchyTreeController::AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platform)
{
	if (hierarchyTree.AddScreen(name, platform))
		EmitHierarchyTreeUpdated();
}

void HierarchyTreeController::CloseProject()
{
	activeControlNodes.clear();
	hierarchyTree.CloseProject();

	ResetSelectedControl();
	UpdateSelection(NULL, NULL);
	EmitHierarchyTreeUpdated();
	emit ProjectClosed();
}

void HierarchyTreeController::DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	if (!nodes.size())
		return;
	
	DeleteNodesInternal(nodes);
	
	UpdateSelection(activePlatform, activeScreen);
	emit SelectedControlNodesChanged(activeControlNodes);
	
	hierarchyTree.DeleteNodes(nodes);
	EmitHierarchyTreeUpdated();
}

void HierarchyTreeController::DeleteNodesInternal(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	const HierarchyTreePlatformNode* activePlatform = this->activePlatform;
	const HierarchyTreeScreenNode* activeScreen = this->activeScreen;
	
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = nodes.begin(); iter != nodes.end(); ++iter)
	{
		if (activeScreen && activeScreen->GetId() == (*iter)->GetId())
			activeScreen = NULL;
		if (activePlatform && activePlatform->GetId() == (*iter)->GetId())
			activePlatform = NULL;
		
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>((*iter));
		if (controlNode)
		{
			UnselectControl(controlNode);
			DeleteNodesInternal(controlNode->GetChildNodes());
		}
	}
}

void HierarchyTreeController::UpdateLocalization(bool takePathFromLocalizationSystem)
{
    // Update the Active Platform.
    HierarchyTreePlatformNode* activePlatformNode = GetActivePlatform();
    if (!activePlatformNode)
    {
        return;
    }

    if (takePathFromLocalizationSystem)
    {
        // FROM LocalizationSystem TO Active Platform
        activePlatformNode->SetLocalizationPath(LocalizationSystem::Instance()->GetDirectoryPath());
        activePlatformNode->SetLocale(LocalizationSystem::Instance()->GetCurrentLocale());
    }
    else
    {
        // FROM Active Platform TO Localization System.
        const String& localizationPath = activePlatformNode->GetLocalizationPath();
        const String& locale = activePlatformNode->GetLocale();

        if (localizationPath.empty() || locale.empty())
        {
            // No Localization Path is already set - cleanup the Localization System.
            LocalizationSystem::Instance()->Cleanup();
        }
        else
        {
            // Re-setup the Localization System with the values stored on Platform level.
            LocalizationSystem::Instance()->SetCurrentLocale(locale);
            LocalizationSystem::Instance()->InitWithDirectory(localizationPath);
        }
    }
    
    // Localization System is updated; need to look through all controls
    // and cause them to update their texts according to the new Localization.
    hierarchyTree.UpdateLocalization();
}
