//
//  CommandsController.h
//  UIEditor
//
//  Created by Yuri Coder on 10/25/12.
//
//

#ifndef __UIEditor__CommandsController__
#define __UIEditor__CommandsController__

#include <QObject>

#include "Base/Singleton.h"
#include "BaseCommand.h"
#include "UndoRedoController.h"

namespace DAVA {
    
// Commands Controller - singleton which is responsible for executing
// commands and performing Undo/Redo.
class CommandsController : public QObject, public Singleton<CommandsController>
{
    Q_OBJECT
public:
	explicit CommandsController(QObject* parent = NULL);
	
    void ExecuteCommand(BaseCommand* command);
	
	// Undo/Redo functionality.
	bool IsUndoAvailable();
	bool IsRedoAvailable();

	bool Undo();
	bool Redo();

	void CleanupUndoRedoStack();

    // This method is called by commands if some property values are changed and needs to be
    // updated.
    void EmitUpdatePropertyValues();
    
    // These methods are called when property change is succeeded or failed.
    void EmitChangePropertySucceeded(const QString& propertyName);
    void EmitChangePropertyFailed(const QString& propertyName);

	bool IsLastChangeSaved() const;
	
signals:
    // Called when some command changes the properties, so the values need to be updated.
    void UpdatePropertyValues();
    
    // Called when property change is succeeded/failed.
    void ChangePropertySucceeded(const QString& propertyName);
    void ChangePropertyFailed(const QString& propertyName);
	
	// Called when Undo/Redo availability is changed.
	void UndoRedoAvailabilityChanged();

	// Called when number of unsaved changes is changed
	void UnsavedChangesNumberChanged();

protected slots:
	void OnProjectSaved();
    
protected:
	int32 unsavedChanges;
	
	// Undo/Redo controller.
	UndoRedoController undoRedoController;
};
    
}
#endif /* defined(__UIEditor__CommandsController__) */
