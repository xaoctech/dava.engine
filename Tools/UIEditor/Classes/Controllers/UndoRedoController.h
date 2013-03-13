//
//  UndoRedoController.h
//  UIEditor
//
//  Created by Yuri Coder on 1/2/13.
//
//

#ifndef __UIEditor__UndoRedoController__
#define __UIEditor__UndoRedoController__

#include "DAVAEngine.h"
#include "../Commands/BaseCommand.h"

namespace DAVA
{
// Undo/Redo Controller for UI Editor.
class UndoRedoController
{
public:
	// Maximum size of the Undo/Redo stacks.
	static const uint32 MAX_UNDO_REDO_STACK_SIZE;

	UndoRedoController();
	virtual ~UndoRedoController();

	// Add the command to Undo/Redo stack. Should be called before each execution of command.
	void AddCommandToUndoStack(BaseCommand* command);
	
	// Undo the last command, returns TRUE if succeeded.
	bool Undo();
	
	// Redo the last command, returns TRUE if succeeded.
	bool Redo();
	
	// Can we perform Undo/Redo?
	bool CanUndo();
	bool CanRedo();

	// Cleanup the stacks.
	void Cleanup();

protected:

	// Add command to the particular stack, with checking the stack size.
	void AddCommandToStack(Deque<BaseCommand*>& stackToAdd, BaseCommand* command);

	// Cleanup the particular stack.
	void CleanupStack(Deque<BaseCommand*>& stackToCleanup);

	// Stacks.
	Deque<BaseCommand*> undoStack;
	Deque<BaseCommand*> redoStack;
};

};
#endif /* defined(__UIEditor__UndoRedoController__) */
