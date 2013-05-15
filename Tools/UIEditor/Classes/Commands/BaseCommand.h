//
//  BaseCommand.h
//  UIEditor
//
//  Created by Yuri Coder on 10/22/12.
//
//

#ifndef __UIEditor__BaseCommand__
#define __UIEditor__BaseCommand__

#include <DAVAEngine.h>
#include "BaseMetadata.h"
#include "HierarchyTreeController.h"

namespace DAVA {

// Base Command for all the Commands existing in the UI Editor system.
class BaseCommand: public BaseObject
{
public:
    BaseCommand();
    virtual ~BaseCommand();

    // Execute command.
    virtual void Execute() = 0;
    
    // Rollback command (only for the commands which does support Undo/Redo).
    virtual void Rollback() {};
	
	// Is the Undo/Redo supported for this command?
	virtual bool IsUndoRedoSupported() = 0;
	
	virtual void ActivateCommandScreen();

	// Access to the screen unsaved changes counter.
	virtual void IncrementUnsavedChanges();
	virtual void DecrementUnsavedChanges();
	void ResetUnsavedChanges();

protected:
    // Get the Metadata for the tree node passed.
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID);
	
	HierarchyTreeNode::HIERARCHYTREENODEID activePlatform;
	HierarchyTreeNode::HIERARCHYTREENODEID activeScreen;
};
    
}

#endif /* defined(__UIEditor__BaseCommand__) */
