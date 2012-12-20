//
//  HierarchyTree.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTree.h"
#include "HierarchyTreePlatformNode.h"
#include "HierarchyTreeScreenNode.h"
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeNode.h"
#include "HierarchyTreeController.h"

#include "MetadataFactory.h"
#include "ResourcesManageHelper.h"

#include <QFile>

#define PLATFORMS_NODE "platforms"
#define LOCALIZATION_NODE "localization"
#define LOCALIZATION_PATH_NODE "LocalizationPath"
#define LOCALIZATION_LOCALE_NODE "Locale"

HierarchyTree::HierarchyTree()
{
	projectCreated = false;
}

void HierarchyTree::Clear()
{
    rootNode.Clear();
}

bool HierarchyTree::Load(const QString& projectPath)
{
	CreateProject();
    
/*	HierarchyTreePlatformNode* platformNode = new HierarchyTreePlatformNode(&rootNode, "Platform1");
	platformNode->SetSize(700, 500);
	rootNode.AddTreeNode(platformNode);
	HierarchyTreeScreenNode* screenNode = new HierarchyTreeScreenNode(platformNode, "Screen1");
	platformNode->AddTreeNode(screenNode);
		
	FileSystem::Instance()->ReplaceBundleName("/Users/adebt/Downloads/Project1/platform1/Data/");
	screenNode->Load("/Users/adebt/Downloads/Project1/platform1/Data/UI/Intro.yaml");
	
	platformNode = new HierarchyTreePlatformNode(&rootNode, "Platform2");
    screenNode = new HierarchyTreeScreenNode(platformNode, "Screen1");
    platformNode->AddTreeNode(screenNode);
	platformNode->SetSize(300, 200);
    rootNode.AddTreeNode(platformNode);
	
	HierarchyTreeController::Instance()->UpdateSelection(platformNode, screenNode);*/
		
	YamlParser* project = YamlParser::Create(projectPath.toStdString());
	if (!project)
		return false;
	
	rootNode.SetProjectPath(projectPath);
	
	YamlNode* projectRoot = project->GetRootNode();
	if (!projectRoot)
	{
		SafeRelease(project);
		return false;
	}

    // NO Localization Data should exist at this point, otherwise automatic
    // LocalizedStrings obtaining will interfere with the loading process!
    LocalizationSystem::Instance()->Cleanup();

    Map<HierarchyTreePlatformNode*, YamlNode*> loadedPlatforms;

	bool result = true;
	YamlNode* platforms = projectRoot->Get(PLATFORMS_NODE);
	for (int32 i = 0; i < platforms->GetCount(); i++)
	{
		YamlNode* platform = platforms->Get(i);
		if (!platform)
			continue;
		
		String platformName = platform->AsString();
		HierarchyTreePlatformNode* platformNode = new HierarchyTreePlatformNode(&rootNode, QString::fromStdString(platformName));
		result &= platformNode->Load(platform);
		rootNode.AddTreeNode(platformNode);
        
        // Remember the platform to load its localization later.
        loadedPlatforms.insert(std::make_pair(platformNode, platform));
	}

    // After the project is loaded and tree is build, update the Tree Extradata with the texts from buttons just loaded.
    // Do this for all platforms and screens. The update direction is FROM Control TO Extra Data.
    UpdateExtraData(BaseMetadata::UPDATE_EXTRADATA_FROM_CONTROL);

    // Now we can load the Localization for each Platform.
    for (Map<HierarchyTreePlatformNode*, YamlNode*>::iterator iter = loadedPlatforms.begin();
         iter != loadedPlatforms.end(); iter ++)
    {
        iter->first->LoadLocalization(iter->second);
    }
    
    // All the data needed is loaded.
    SafeRelease(project);

    // Initialize the control names with their correct (localized) values after the
    // Localization File is loaded.
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED);

	HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>((*rootNode.GetChildNodes().begin()));
	HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>((*platformNode->GetChildNodes().begin()));
    
    // After the project is loaded and tree is build, update the Tree Extradata with the texts from buttons just loaded.
    // Do this for all platforms and screens.
    
	HierarchyTreeController::Instance()->UpdateSelection(platformNode, screenNode);

	return result;
}

void HierarchyTree::CreateProject()
{
	projectCreated = true;
}

void HierarchyTree::CloseProject()
{
	projectCreated = false;
	Clear();
}

void HierarchyTree::AddPlatform(const QString& name, const Vector2& size)
{
    HierarchyTreePlatformNode* platformNode = new HierarchyTreePlatformNode(&rootNode, name);
	platformNode->SetSize(size.dx, size.dy);
	rootNode.AddTreeNode(platformNode);
}

bool HierarchyTree::AddScreen(const QString& name, HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	HierarchyTreeNode* baseNode = FindNode(&rootNode, platformId);
	HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(baseNode);
	if (!platformNode)
		return false;
	
	platformNode->AddTreeNode(new HierarchyTreeScreenNode(platformNode, name));
	return true;
}

HierarchyTreeNode* HierarchyTree::GetNode(HierarchyTreeNode::HIERARCHYTREENODEID id) const
{
	return FindNode(&rootNode, id);
}

HierarchyTreeNode* HierarchyTree::FindNode(const HierarchyTreeNode *parent, HierarchyTreeNode::HIERARCHYTREENODEID id) const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = parent->GetChildNodes().begin(); iter != parent->GetChildNodes().end(); ++iter)
    {
		HierarchyTreeNode* node = (*iter);
		
		if (node->GetId() == id)
			return node;
		
		node = FindNode(node, id);
		if (node)
			return node;
	}
	
	return NULL;
}

HierarchyTreeNode* HierarchyTree::GetNode(const UIControl* control) const
{
	return FindNode(&rootNode, control);
}

HierarchyTreeNode* HierarchyTree::FindNode(const HierarchyTreeNode* parent, const UIControl* control) const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter;
	for (iter = parent->GetChildNodes().begin(); iter != parent->GetChildNodes().end(); ++iter) {
		HierarchyTreeNode* node = (*iter);
		
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode && controlNode->GetUIObject() == control)
			return node;
		
		node = FindNode(node, control);
		if (node)
			return node;
	}
	
	return NULL;
}

const HierarchyTreeNode::HIERARCHYTREENODESLIST& HierarchyTree::GetPlatforms() const
{
	return rootNode.GetChildNodes();
}

void HierarchyTree::DeleteNodes(const HierarchyTreeNode::HIERARCHYTREENODESLIST& nodes)
{
	//copy id for safe delete
	Set<HierarchyTreeControlNode::HIERARCHYTREENODEID> ids;
	Set<HierarchyTreeControlNode::HIERARCHYTREENODEID>::iterator idIter;
	HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator nodeIter;
	for (nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter)
	{
		ids.insert((*nodeIter)->GetId());
	}
	
	for (idIter = ids.begin(); idIter != ids.end(); ++idIter)
	{
		HierarchyTreeControlNode::HIERARCHYTREENODEID id = (*idIter);
		HierarchyTreeNode* node = FindNode(&rootNode, id);
		if (!node)
			continue;
		
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode)
		{
			controlNode->GetParent()->RemoveTreeNode(controlNode);
			continue;
		}
		
		HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(node);
		if (screenNode)
		{
			screenNode->GetPlatform()->RemoveTreeNode(screenNode);
			continue;
		}
		
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(node);
		if (platformNode)
		{
			rootNode.RemoveTreeNode(platformNode);
			continue;
		}
	}
}

bool HierarchyTree::Save(const QString& projectPath)
{
	bool result = true;
	YamlNode root(YamlNode::TYPE_MAP);
	Map<String, YamlNode*> &rootMap = root.AsMap();
	YamlNode* platforms = new YamlNode(YamlNode::TYPE_MAP);
	rootMap[PLATFORMS_NODE] = platforms;

    // Prior to Save we need to put the Localization Keys FROM the ExtraData TO the
    // appropriate text controls to save the localization keys, and not values.
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_RAW);

	QString oldPath = rootNode.GetProjectPath();
	rootNode.SetProjectPath(projectPath);
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = rootNode.GetChildNodes().begin();
		 iter != rootNode.GetChildNodes().end();
		 ++iter)
	{
		HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*iter);
		if (!platformNode)
			continue;
		
		result &= platformNode->Save(platforms);
	}

	YamlParser* parser = YamlParser::Create();
	result &= parser->SaveToYamlFile(projectPath.toStdString(), &root, true);
	
    // Return the Localized Values.
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED);

	if (!result)
	{
		//restore project path
		rootNode.SetProjectPath(oldPath);
	}

	return result;
}

void HierarchyTree::UpdateExtraData(BaseMetadata::eExtraDataUpdateStyle updateStyle)
{
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER platformNodesIter = rootNode.GetChildNodes().begin();
         platformNodesIter != rootNode.GetChildNodes().end(); platformNodesIter ++)
    {
        HierarchyTreePlatformNode* platformNode = dynamic_cast<HierarchyTreePlatformNode*>(*platformNodesIter);
        DVASSERT(platformNode);
        if (!platformNode)
        {
            continue;
        }
        
        for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER screenNodesIter = platformNode->GetChildNodes().begin();
             screenNodesIter != platformNode->GetChildNodes().end(); screenNodesIter ++)
        {
            HierarchyTreeScreenNode* screenNode = dynamic_cast<HierarchyTreeScreenNode*>(*screenNodesIter);
            DVASSERT(screenNode);
            if (!screenNode)
            {
                continue;
            }

                // Update extra data from controls in a recursive way.
            for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER controlNodesIter = screenNode->GetChildNodes().begin();
                 controlNodesIter != screenNode->GetChildNodes().end(); controlNodesIter ++)
            {
                HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(*controlNodesIter);
                UpdateExtraDataRecursive(controlNode, updateStyle);
            }
        }
    }
}

void HierarchyTree::UpdateExtraDataRecursive(HierarchyTreeControlNode* node, BaseMetadata::eExtraDataUpdateStyle updateStyle)
{
    if (!node)
    {
        DVASSERT(false);
        return;
    }

    // Create the Metadata, initialize with params.
    BaseMetadata* metadata = MetadataFactory::Instance()->GetMetadataForTreeNode(node);
    if (metadata)
    {
        METADATAPARAMSVECT params;
        params.push_back(BaseMetadataParams(node->GetId(), node->GetUIObject()));
        metadata->SetupParams(params);
        metadata->SetActiveParamID(0);

        metadata->UpdateExtraData(node->GetExtraData(), updateStyle);
    }

    // Repeat the same for all inner children.
    for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = node->GetChildNodes().begin();
         iter != node->GetChildNodes().end(); iter ++)
    {
        HierarchyTreeControlNode* childNode = dynamic_cast<HierarchyTreeControlNode*>(*iter);
        DVASSERT(childNode);
        if (!childNode)
        {
            continue;
        }
        
        UpdateExtraDataRecursive(childNode, updateStyle);
    }
}

void HierarchyTree::UpdateLocalization()
{
    UpdateExtraData(BaseMetadata::UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED);
}