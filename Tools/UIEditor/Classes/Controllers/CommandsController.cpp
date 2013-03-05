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
	unsavedChanges = 0;
	
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
	++unsavedChanges;

	SafeRetain(command);
    command->Execute();

	undoRedoController.AddCommandToUndoStack(command);
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

bool CommandsController::IsLastChangeSaved() const
{
	return (unsavedChanges == 0);
}

void CommandsController::OnProjectSaved()
{
	unsavedChanges = 0;
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
	--unsavedChanges;

	bool undoResult = undoRedoController.Undo();
	emit UndoRedoAvailabilityChanged();
	emit UnsavedChangesNumberChanged();
	return undoResult;
}

bool CommandsController::Redo()
{
	++unsavedChanges;

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
