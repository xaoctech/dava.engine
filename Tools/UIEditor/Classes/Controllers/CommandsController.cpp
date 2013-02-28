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
	isLastChangeSaved = true;
	
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
	isLastChangeSaved = false;

	SafeRetain(command);
    command->Execute();

	undoRedoController.AddCommandToUndoStack(command);
	emit UndoRedoAvailabilityChanged();

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
	return isLastChangeSaved;
}

void CommandsController::OnProjectSaved()
{
	isLastChangeSaved = true;
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
	bool undoResult = undoRedoController.Undo();
	emit UndoRedoAvailabilityChanged();
	return undoResult;
}

bool CommandsController::Redo()
{
	bool redoResult = undoRedoController.Redo();
	emit UndoRedoAvailabilityChanged();
	return redoResult;
}

void CommandsController::CleanupUndoRedoStack()
{
	undoRedoController.Cleanup();
	emit UndoRedoAvailabilityChanged();
}
