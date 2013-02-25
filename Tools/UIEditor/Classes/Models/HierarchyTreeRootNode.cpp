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

void HierarchyTreeRootNode::SetProjectFilePath(const QString& projectFilePath)
{
	this->projectFilePath = projectFilePath;
	// Save project directory path
	QFileInfo fileInfo(projectFilePath);
	this->projectDir = fileInfo.absoluteDir().absolutePath();
}
