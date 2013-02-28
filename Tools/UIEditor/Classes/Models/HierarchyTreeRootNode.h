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
	
	const QString& GetProjectDir() const {return projectDir;};
	const QString& GetProjectFilePath() const {return projectFilePath;};
	void SetProjectFilePath(const QString& projectFilePath);
	
private:
	QString projectFilePath;
	QString projectDir;
};

#endif /* defined(__UIEditor__HierarchyTreeRootNode__) */
