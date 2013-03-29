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
		PasteCommand(HierarchyTreeNode* parentNode, CopyPasteController::CopyType copyType, const HierarchyTreeNode::HIERARCHYTREECOPYNODESLIST * items);
		virtual ~PasteCommand();
		
		// Execute command.
		virtual void Execute();
		virtual void Rollback();
		virtual bool IsUndoRedoSupported() {return true;};
		
	private:
        int PasteControls(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreeNode *parent);
		int PasteScreens(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreePlatformNode* parent);
		int PastePlatforms(HierarchyTreeNode::HIERARCHYTREENODESLIST*, HierarchyTreeRootNode* parent);
		
		QString FormatCopyName(QString baseName, const HierarchyTreeNode* parent) const;
		
		void UpdateControlName(const HierarchyTreeNode* parent, HierarchyTreeNode* node, bool needCreateNewName) const;
		
		// Undo/Redo-related functionality.
		void ReturnPastedControlsToScene();
		void CleanupPastedItems();

	private:
		HierarchyTreeNode* parentNode;
		CopyPasteController::CopyType copyType;
		
		// Items to be pasted.
		const HierarchyTreeNode::HIERARCHYTREECOPYNODESLIST* items;
		
		// Items were pasted (coy of the items to be pasted).
		HierarchyTreeNode::HIERARCHYTREENODESLIST* newItems;
	};
}

#endif /* defined(__UIEditor__PasteCommand__) */
