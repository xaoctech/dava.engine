//
//  CommandsController.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/25/12.
//
//

#include "CommandsController.h"
#include "Base/BaseTypes.h"

using namespace DAVA;

CommandsController::CommandsController(QObject* parent/* = NULL*/) :
	QObject(parent)
{
	//We should reset isLastChangeSaved flag each time the project was closed
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectClosed()),
			this,
			SLOT(OnProjectSaved()));
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(ProjectSaved()),
			this,
			SLOT(OnProjectSaved()));
}

void CommandsController::ExecuteCommand(BaseCommand* command)
{
	SafeRetain(command);
    command->Execute();

	undoRedoController.AddCommandToUndoStack(command);
	undoRedoController.IncrementUnsavedChanges(true);

	emit UndoRedoAvailabilityChanged();
	emit UnsavedChangesNumberChanged();

	SafeRelease(command);
}

void CommandsController::EmitUpdatePropertyValues()
{
    emit UpdatePropertyValues();
}

void CommandsController::EmitChangePropertySucceeded(const QString& propertyName)
{
    emit ChangePropertySucceeded(propertyName);
}

void CommandsController::EmitChangePropertyFailed(const QString& propertyName)
{
    emit ChangePropertyFailed(propertyName);
}

void CommandsController::OnProjectSaved()
{
	emit UnsavedChangesNumberChanged();
}

// Undo/Redo functionality.
bool CommandsController::IsUndoAvailable()
{
	return undoRedoController.CanUndo();
}

bool CommandsController::IsRedoAvailable()
{
	return undoRedoController.CanRedo();
}

bool CommandsController::Undo()
{
	undoRedoController.DecrementUnsavedChanges(true);
	bool undoResult = undoRedoController.Undo();

	emit UndoRedoAvailabilityChanged();
	emit UnsavedChangesNumberChanged();
	return undoResult;
}

bool CommandsController::Redo()
{
	undoRedoController.IncrementUnsavedChanges(false);
	bool redoResult = undoRedoController.Redo();

	emit UndoRedoAvailabilityChanged();
	emit UnsavedChangesNumberChanged();
	return redoResult;
}

void CommandsController::CleanupUndoRedoStack()
{
	undoRedoController.Cleanup();
	emit UndoRedoAvailabilityChanged();
}
