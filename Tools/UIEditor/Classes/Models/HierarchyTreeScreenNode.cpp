//
//  HierarchyTreeScreenNode.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreeScreenNode.h"
#include "ScreenManager.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeAggregatorControlNode.h"
#include <QFile>
#include "Render/2D/FontManager.h"

HierarchyTreeScreenNode::HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const QString& name) :
	HierarchyTreeNode(name)
{
	this->parent = parent;
	this->screen = new ScreenControl();
	screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
	
	scale = 1.f;
	posX = 0;
	posY = 0;
}

HierarchyTreeScreenNode::HierarchyTreeScreenNode(HierarchyTreePlatformNode* parent, const HierarchyTreeScreenNode* base):
	HierarchyTreeNode(base)
{
	this->parent = parent;
	this->screen = new ScreenControl();
	if (parent)
		screen->SetRect(Rect(0, 0, parent->GetWidth(), parent->GetHeight()));
	
	scale = 1.f;
	posX = 0;
	posY = 0;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& chilren = base->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = chilren.begin();
		 iter != chilren.end();
		 ++iter)
	{
		const HierarchyTreeControlNode* baseControl = dynamic_cast<const HierarchyTreeControlNode* >((*iter));
		if (!baseControl)
			continue;
		
		HierarchyTreeControlNode* control = new HierarchyTreeControlNode(this, baseControl);
		AddTreeNode(control);
	}
}

HierarchyTreeScreenNode::~HierarchyTreeScreenNode()
{
	Cleanup();
	SafeRelease(screen);
}

void HierarchyTreeScreenNode::SetScale(float scale)
{
	this->scale = scale;
}

float HierarchyTreeScreenNode::GetScale() const
{
	return scale;
}

void HierarchyTreeScreenNode::SetPosX(int x)
{
	this->posX = x;
}

int HierarchyTreeScreenNode::GetPosX() const
{
	return posX;
}

void HierarchyTreeScreenNode::SetPosY(int y)
{
	this->posY = y;
}

int HierarchyTreeScreenNode::GetPosY() const
{
	return posY;
}

void HierarchyTreeScreenNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	HierarchyTreePlatformNode* newPlatform = dynamic_cast<HierarchyTreePlatformNode*>(node);
	DVASSERT(newPlatform);
	if (!newPlatform)
		return;
	
	HierarchyTreePlatformNode* oldPlatform = GetPlatform();
	if (oldPlatform)
	{
		oldPlatform->RemoveTreeNode(this, false, false);
	}
	
	parent = newPlatform;
	GetScreen()->SetRect(Rect(0, 0, newPlatform->GetWidth(), newPlatform->GetHeight()));
	newPlatform->AddTreeNode(this, insertAfter);
}

HierarchyTreeNode* HierarchyTreeScreenNode::GetParent()
{
	return this->parent;
}

String HierarchyTreeScreenNode::GetNewControlName(const String& baseName) const
{
	int i = 0;
	while (true)
	{
		QString newName = QString().fromStdString(baseName) + QString::number(++i);
		
		if (!IsNameExist(newName, this))
			return newName.toStdString();
	}
}
			
bool HierarchyTreeScreenNode::IsNameExist(const QString &name, const HierarchyTreeNode *parent) const
{
	if (!parent)
		return false;
	
	const HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HIERARCHYTREENODESLIST::const_iterator iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		if (node->GetName().compare(name) == 0)
			return true;

		if (IsNameExist(name, node))
			return true;
	}
	
	return false;
}

bool HierarchyTreeScreenNode::Load(const QString& path)
{
	UIYamlLoader loader;
	loader.Load(screen, path.toStdString());
	
	BuildHierarchyTree(this, screen->GetChildren());
	return true;
}

void HierarchyTreeScreenNode::BuildHierarchyTree(HierarchyTreeNode* parent, List<UIControl*> child)
{
	for (List<UIControl*>::const_iterator iter = child.begin();
		 iter != child.end();
		 ++iter)
	{
		UIControl* uiControl = (*iter);
		
		HierarchyTreeControlNode* node = NULL;
		if (dynamic_cast<UIAggregatorControl*>(uiControl))
			node = new HierarchyTreeAggregatorControlNode(NULL, parent, uiControl, QString::fromStdString(uiControl->GetName()));
		else
			node = new HierarchyTreeControlNode(parent, uiControl, QString::fromStdString(uiControl->GetName()));

		// Use the "real" children list here to avoid loading of (for example) separate Static Text for UITextControls.
		BuildHierarchyTree(node, uiControl->GetRealChildren());
		parent->AddTreeNode(node);
	}
}

bool HierarchyTreeScreenNode::Save(const QString& path)
{
	FontManager::Instance()->PrepareToSaveFonts();
	return UIYamlLoader::Save(screen, path.toStdString(), true);
}

void HierarchyTreeScreenNode::ReturnTreeNodeToScene()
{
	if (!this->redoParentNode)
	{
		return;
	}
	
	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);
}

void HierarchyTreeScreenNode::CombineRectWithChild(Rect& rect) const
{
	const HIERARCHYTREENODESLIST& childs = GetChildNodes();
	for (HIERARCHYTREENODESLIST::const_iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		const HierarchyTreeControlNode* control = dynamic_cast<const HierarchyTreeControlNode*>(*iter);
		if (!control)
			continue;
		
		Rect controlRect = control->GetRect();
		
		rect = rect.Combine(controlRect);
	}
}

Rect HierarchyTreeScreenNode::GetRect() const
{
	Rect rect(0, 0, GetPlatform()->GetWidth(), GetPlatform()->GetHeight());
	
	CombineRectWithChild(rect);
	
	return rect;
}
