//
//  PasteCommand.h
//  UIEditor
//
//  Created by adebt on 11/5/12.
//
//

#ifndef __UIEditor__PasteCommand__
#define __UIEditor__PasteCommand__

#include "BaseCommand.h"
#include "CopyPasteController.h"

namespace DAVA
{
	class PasteCommand: public BaseCommand
	{
	public:
		PasteCommand(HierarchyTreeNode* parentNode, CopyPasteController::CopyType copyType, const HierarchyTreeNode::HIERARCHYTREENODESLIST* items);
		virtual ~PasteCommand();
		
		// Execute command.
		virtual void Execute();
		virtual bool IsUndoRedoSupported() {return false;};
		
	private:
        int PasteControls(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreeNode *parent);
		int PasteScreens(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreePlatformNode* parent);
		int PastePlatforms(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreeRootNode* parent);
		
		QString FormatCopyName(QString baseName, const HierarchyTreeNode* parent) const;
		
		void UpdateControlName(const HierarchyTreeNode* parent, HierarchyTreeNode* node, bool needCreateNewName) const;
		
	private:
		HierarchyTreeNode* parentNode;
		CopyPasteController::CopyType copyType;
		const HierarchyTreeNode::HIERARCHYTREENODESLIST* items;
	};
}

#endif /* defined(__UIEditor__PasteCommand__) */
