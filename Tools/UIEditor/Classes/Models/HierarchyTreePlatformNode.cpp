//
//  HierarchyTreePlatformNode.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeAggregatorNode.h"
#include "ResourcesManageHelper.h"
#include <QDir>

#define WIDTH_NODE "width"
#define HEIGHT_NODE "height"
#define SCREENS_NODE "screens"
#define AGGREGATORS_NODE "aggregators"

#define LOCALIZATION_PATH_NODE "LocalizationPath"
#define LOCALIZATION_LOCALE_NODE "Locale"

#define SCREEN_PATH "%1/%2.yaml"

HierarchyTreePlatformNode::HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const QString& name) :
	HierarchyTreeNode(name)
{
	this->rootNode = rootNode;
	width = height = 0;
}

HierarchyTreePlatformNode::HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const HierarchyTreePlatformNode* base) :
	HierarchyTreeNode(base)
{
	this->rootNode = rootNode;
	this->width = base->GetWidth();
	this->height = base->GetHeight();
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& chilren = base->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = chilren.begin();
		 iter != chilren.end();
		 ++iter)
	{
		const HierarchyTreeScreenNode* baseControl = dynamic_cast<const HierarchyTreeScreenNode* >((*iter));
		if (!baseControl)
			continue;
		
		HierarchyTreeScreenNode* control = new HierarchyTreeScreenNode(this, baseControl);
		AddTreeNode(control);
	}
}

void HierarchyTreePlatformNode::SetSize(int width, int height)
{
	this->width = width;
	this->height = height;
	
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (aggregator)
			continue;
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (screenNode)
		{
			screenNode->GetScreen()->SetRect(Rect(0, 0, width, height));
		}
	}
}
	
int HierarchyTreePlatformNode::GetWidth() const
{
	return width;
}

int HierarchyTreePlatformNode::GetHeight() const
{
	return height;
}

HierarchyTreeNode* HierarchyTreePlatformNode::GetParent()
{
	// Root Node is the parent for the Platform.
	return this->rootNode;
}

QString HierarchyTreePlatformNode::GetPlatformFolder() const
{
	QString path;
	if (rootNode)
	{
		path = ResourcesManageHelper::GetPlatformRootPath(rootNode->GetProjectDir());
	}
	path += GetName();

	return path;
}

void HierarchyTreePlatformNode::ActivatePlatform()
{
	if (rootNode)
	{
		String bundleName = ResourcesManageHelper::GetDataPath(rootNode->GetProjectDir()).toStdString();
		FileSystem::Instance()->ReplaceBundleName(bundleName);
	}
}

bool HierarchyTreePlatformNode::Load(YamlNode* platform)
{
	YamlNode* width = platform->Get(WIDTH_NODE);
	YamlNode* height = platform->Get(HEIGHT_NODE);
	if (!width || !height)
		return false;
	
	bool result = true;
	SetSize(width->AsInt(), height->AsInt());
	ActivatePlatform();
	
	YamlNode* screens = platform->Get(SCREENS_NODE);
	if (screens)
	{
		for (int i = 0; i < screens->GetCount(); i++)
		{
			YamlNode* screen = screens->Get(i);
			if (!screen)
				continue;
			String screenName = screen->AsString();
			
			QString screenPath = QString(SCREEN_PATH).arg(GetPlatformFolder()).arg(QString::fromStdString(screenName));
			HierarchyTreeScreenNode* screenNode = new HierarchyTreeScreenNode(this, QString::fromStdString(screenName));
			result &= screenNode->Load(screenPath);
			AddTreeNode(screenNode);
		}
	}
	
	YamlNode* aggregators = platform->Get(AGGREGATORS_NODE);
	if (aggregators)
	{
		for (int i = 0; i < aggregators->GetCount(); i++)
		{
			YamlNode* aggregator = aggregators->Get(i);
			if (!aggregator)
				continue;
			String aggregatorName = aggregator->AsString();
			
			QString aggregatorPath = QString(SCREEN_PATH).arg(GetPlatformFolder()).arg(QString::fromStdString(aggregatorName));
			HierarchyTreeAggregatorNode* aggregatorNode = new HierarchyTreeAggregatorNode(this, QString::fromStdString(aggregatorName), Rect());
			result &= aggregatorNode->Load(aggregator, aggregatorPath);
			AddTreeNode(aggregatorNode);			
		}
	}
	
	return result;
}

bool HierarchyTreePlatformNode::LoadLocalization(YamlNode* platform)
{
    if (!platform)
    {
        return false;
    }
    
    YamlNode* pathNode = platform->Get(LOCALIZATION_PATH_NODE);
    YamlNode* localeNode = platform->Get(LOCALIZATION_LOCALE_NODE);

    if (pathNode && localeNode &&
        !pathNode->AsString().empty() &&
        !localeNode->AsString().empty())
    {
        localizationPath = pathNode->AsString();
        locale = localeNode->AsString();
    }

    return true;
}

bool HierarchyTreePlatformNode::Save(YamlNode* node, bool saveAll)
{
	YamlNode* platform = new YamlNode(YamlNode::TYPE_MAP);
	platform->Set(WIDTH_NODE, GetWidth());
	platform->Set(HEIGHT_NODE, GetHeight());

	MultiMap<String, YamlNode*> &platformsMap = node->AsMap();
	platformsMap.erase(GetName().toStdString());
	platformsMap.insert(std::pair<String, YamlNode*>(GetName().toStdString(), platform));
	ActivatePlatform();
	
	MultiMap<String, YamlNode*> &platformMap = platform->AsMap();
	YamlNode* screens = new YamlNode(YamlNode::TYPE_ARRAY);
	platformMap.erase(SCREENS_NODE);
	platformMap.insert(std::pair<String, YamlNode*>(SCREENS_NODE, screens));
	
	YamlNode* aggregators = new YamlNode(YamlNode::TYPE_MAP);
	platformMap.erase(AGGREGATORS_NODE);
	platformMap.insert(std::pair<String, YamlNode*>(AGGREGATORS_NODE, aggregators));

    // Add the Localization info - specific for each Platform.
    SaveLocalization(platform);

	QString platformFolder = GetPlatformFolder();

	QDir dir;
	dir.mkpath(platformFolder);
	
	bool result = true;
	
	//save aggregators node before save screens
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (!node)
			continue;

		QString path = QString(SCREEN_PATH).arg(platformFolder).arg(node->GetName());
		MultiMap<String, YamlNode*> &aggregatorsMap = aggregators->AsMap();
		
		YamlNode* aggregator = new YamlNode(YamlNode::TYPE_MAP);
		result &= node->Save(aggregator, path, saveAll);
		
		aggregatorsMap.erase(node->GetName().toStdString());
		aggregatorsMap.insert(std::pair<String, YamlNode*>(node->GetName().toStdString(), aggregator));
	}
		
	
	for (HIERARCHYTREENODESCONSTITER iter = GetChildNodes().begin();
		 iter != GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreeAggregatorNode* node = dynamic_cast<HierarchyTreeAggregatorNode*>(*iter);
		if (node)
			continue;	//skip aggregators
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*iter);
		if (!screenNode)
			continue;
		
		QString screenPath = QString(SCREEN_PATH).arg(platformFolder).arg(screenNode->GetName());
		result &= screenNode->Save(screenPath, saveAll);
		
		screens->AddValueToArray(screenNode->GetName().toStdString());
	}
	return result;
}

bool HierarchyTreePlatformNode::SaveLocalization(YamlNode* platform)
{
    if (!platform)
    {
        return false;
    }

    platform->Set(LOCALIZATION_PATH_NODE, this->localizationPath);
    platform->Set(LOCALIZATION_LOCALE_NODE, locale);

    return true;
}

void HierarchyTreePlatformNode::SetLocalizationPath(const String& localizationPath)
{
    this->localizationPath = localizationPath;
}

void HierarchyTreePlatformNode::SetLocale(const String& locale)
{
    this->locale = locale;
}

void HierarchyTreePlatformNode::ReturnTreeNodeToScene()
{
	if (!this->redoParentNode)
	{
		return;
	}
	
	// Need to recover the node previously deleted, taking position into account.
	this->redoParentNode->AddTreeNode(this, redoPreviousNode);
}

void HierarchyTreePlatformNode::SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter)
{
	if (!node)
		return;
	
	node->RemoveTreeNode(this, false, false);
	node->AddTreeNode(this, insertAfter);
}

bool HierarchyTreePlatformNode::IsAggregatorNamePresent(const QString& candidatName)
{
	for (HIERARCHYTREENODESLIST::iterator iter = childNodes.begin(); iter != childNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeAggregatorNode* aggregator = dynamic_cast<HierarchyTreeAggregatorNode*>(node);
		if (NULL == aggregator)
		{
			continue;
		}
		if(aggregator->GetName().compare(candidatName) == 0)
		{
			return true;
		}

	}
	return false;
}

