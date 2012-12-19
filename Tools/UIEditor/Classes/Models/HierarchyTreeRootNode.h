//
//  HierarchyTreeRootNode.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__HierarchyTreeRootNode__
#define __UIEditor__HierarchyTreeRootNode__

#include "HierarchyTreeNode.h"

using namespace DAVA;

// Root node for the Hierarchy Tree.
class HierarchyTreeRootNode: public HierarchyTreeNode
{
public:
	HierarchyTreeRootNode();
    void Clear();
	
	const QString& GetProjectFolder() const {return projectFolder;};
	const QString& GetProjectPath() const {return projectPath;};
	void SetProjectPath(const QString& projectPath);
	
private:
	QString projectPath;
	QString projectFolder;
};

#endif /* defined(__UIEditor__HierarchyTreeRootNode__) */
