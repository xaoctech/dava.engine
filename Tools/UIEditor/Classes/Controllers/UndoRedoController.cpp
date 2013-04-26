//
//  UndoRedoController.cpp
//  UIEditor
//
//  Created by Yuri Coder on 1/2/13.
//
//

#include "UndoRedoController.h"
using namespace DAVA;

const uint32 UndoRedoController::MAX_UNDO_REDO_STACK_SIZE = 20;

UndoRedoController::UndoRedoController()
{
}

UndoRedoController::~UndoRedoController()
{
	Cleanup();
}

void UndoRedoController::Cleanup()
{
	CleanupStack(undoStack);
	CleanupStack(redoStack);
}

void UndoRedoController::CleanupStack(Deque<BaseCommand*>& stackToCleanup)
{
	while (!stackToCleanup.empty())
	{
		BaseCommand* command = stackToCleanup.front();
		SafeDelete(command);
		stackToCleanup.pop_front();
	}
}

void UndoRedoController::AddCommandToUndoStack(BaseCommand* command)
{
	// Store only the commands which support Undo/Redo.
	if (command->IsUndoRedoSupported() == false)
	{
		return;
	}

	SafeRetain(command);
	AddCommandToStack(undoStack, command);
	
	// Adding the Undo command should cleanup the Redo stack.
	CleanupStack(redoStack);
}

bool UndoRedoController::Undo()
{
	if (!CanUndo())
	{
		return false;
	}
	
	BaseCommand* command = undoStack.front();
	AddCommandToStack(redoStack, command);
	undoStack.pop_front();

	command->ActivateCommandScreen();
	command->Rollback();
	return true;
}

// Redo the last command, returns TRUE if succeeded.
bool UndoRedoController::Redo()
{
	if (!CanRedo())
	{
		return false;
	}
	
	BaseCommand* command = redoStack.front();
	AddCommandToStack(undoStack, command);
	redoStack.pop_front();
	
	command->ActivateCommandScreen();
	command->Execute();
	return true;
}

// Can we perform Undo/Redo?
bool UndoRedoController::CanUndo()
{
	return !undoStack.empty();
}

bool UndoRedoController::CanRedo()
{
	return !redoStack.empty();
}

void UndoRedoController::AddCommandToStack(Deque<BaseCommand*>& stackToAdd, BaseCommand* command)
{
	stackToAdd.push_front(command);
	if (stackToAdd.size() > MAX_UNDO_REDO_STACK_SIZE)
	{
		BaseCommand* commandToRemove = stackToAdd.back();
		SafeDelete(commandToRemove);
		stackToAdd.pop_back();
	}
}

void UndoRedoController::IncrementUnsavedChanges(bool forUndoStack)
{
	Deque<BaseCommand*>& activeStack = forUndoStack ? undoStack : redoStack;
	if (activeStack.empty())
	{
		return;
	}
	
	activeStack.front()->IncrementUnsavedChanges();
}

void UndoRedoController::DecrementUnsavedChanges(bool forUndoStack)
{
	Deque<BaseCommand*>& activeStack = forUndoStack ? undoStack : redoStack;
	if (activeStack.empty())
	{
		return;
	}
	
	activeStack.front()->DecrementUnsavedChanges();
}

void UndoRedoController::ResetUnsavedChanges()
{
	// TODO! IMPLEMENT!
}
