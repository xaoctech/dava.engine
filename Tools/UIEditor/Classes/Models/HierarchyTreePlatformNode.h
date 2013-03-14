//
//  HierarchyTreePlatformNode.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTreePlatformNode__
#define __UIEditor__HierarchyTreePlatformNode__

#include "HierarchyTreeNode.h"
#include "HierarchyTreeRootNode.h"

using namespace DAVA;
    
// "Platform" node for the Hierarchy Tree.
class HierarchyTreePlatformNode: public HierarchyTreeNode
{
public:
	HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const QString& name);
	HierarchyTreePlatformNode(HierarchyTreeRootNode* rootNode, const HierarchyTreePlatformNode* base);

	void SetSize(int width, int height);
	int GetWidth() const;
	int GetHeight() const;

	virtual HierarchyTreeNode* GetParent();

	HierarchyTreeNode* GetRoot() {return rootNode;};
	
	QString GetPlatformFolder() const;
	void ActivatePlatform();
	
	bool Load(YamlNode* node);
	bool Save(YamlNode* node);

    // Separate method to load/save localization.
    bool LoadLocalization(YamlNode* platform);
    bool SaveLocalization(YamlNode* platform);

    // Accessors to the current localization info.
    const String& GetLocalizationPath() const {return localizationPath;};
    const String& GetLocale() const {return locale;};
    
    void SetLocalizationPath(const String& localizationPath);
    void SetLocale(const String& locale);
    
	// Return the Platform Node back to scene after deletion when performing Undo.
	virtual void ReturnTreeNodeToScene();
	
	virtual void SetParent(HierarchyTreeNode* node, HierarchyTreeNode* insertAfter);

private:
	int width;
	int height;

    String localizationPath;
    String locale;

	HierarchyTreeRootNode* rootNode;
};

#endif /* defined(__UIEditor__HierarchyTreePlatformNode__) */
