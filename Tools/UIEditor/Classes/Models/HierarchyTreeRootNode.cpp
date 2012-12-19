//
//  HierarchyTreeRootNode.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "HierarchyTreeRootNode.h"
#include <QFileInfo>
#include <QDir>

HierarchyTreeRootNode::HierarchyTreeRootNode() :
	HierarchyTreeNode("")
{
	
}

void HierarchyTreeRootNode::Clear()
{
    Cleanup();
}

void HierarchyTreeRootNode::SetProjectPath(const QString& projectPath)
{
	this->projectPath = projectPath;
	QFileInfo fileInfo(projectPath);
	projectFolder = fileInfo.absoluteDir().absolutePath() + "/";
}
