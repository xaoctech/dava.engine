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
}

bool UndoRedoController::Undo()
{
	Logger::Debug("BEFORE UNDO: undo stack size %i, redo stack size %i", undoStack.size(), redoStack.size());
	if (!CanUndo())
	{
		return false;
	}
	
	BaseCommand* command = undoStack.front();
	AddCommandToStack(redoStack, command);
	undoStack.pop_front();

	command->Rollback();
	Logger::Debug("AFTER UNDO: undo stack size %i, redo stack size %i", undoStack.size(), redoStack.size());

	return true;
}

// Redo the last command, returns TRUE if succeeded.
bool UndoRedoController::Redo()
{
	Logger::Debug("BEFORE REDO: undo stack size %i, redo stack size %i", undoStack.size(), redoStack.size());
	if (!CanRedo())
	{
		return false;
	}
	
	BaseCommand* command = redoStack.front();
	AddCommandToStack(undoStack, command);
	redoStack.pop_front();
	
	command->Execute();
	Logger::Debug("AFTER REDO: undo stack size %i, redo stack size %i", undoStack.size(), redoStack.size());
	
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

