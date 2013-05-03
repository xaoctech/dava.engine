#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "DAVAEngine.h"
#include "CommandList.h"

class MultiCommand;
class CommandsManager;
class Command: public DAVA::BaseObject
{
	friend class MultiCommand;
    friend class CommandsManager;
    
protected:
    enum eCommandState
    {
        STATE_VALID = 0,
        STATE_INVALID
    };
    
    
public:
    
    enum eCommandType
    {
        COMMAND_WITHOUT_UNDO_EFFECT = 0,    // Command has no effect onto undo-redo queue
        COMMAND_CLEAR_UNDO_QUEUE,           // Command reset undo queue
        COMMAND_UNDO_REDO                   // You cat undo & redo this command
    };
    
public:
	Command(eCommandType _type, CommandList::eCommandId id);
	virtual ~Command();

protected:	
    
    virtual void Execute() = 0;
    virtual void Cancel() {};

	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

    inline eCommandType Type() const {return commandType; };
	inline CommandList::eCommandId Id() const {return commandId; };
	
    inline void SetState(eCommandState newState) {commandState = newState; };
    inline eCommandState State() const {return commandState; };

protected:
    
    eCommandType commandType;
    eCommandState commandState;
	CommandList::eCommandId commandId;

	DAVA::String commandName;
};

class MultiCommand: public Command
{
public:
	MultiCommand(eCommandType _type, CommandList::eCommandId id);

protected:
	void ExecuteInternal(Command* command);
	void CancelInternal(Command* command);

	DAVA::Set<DAVA::Entity*> GetAffectedEntitiesInternal(Command* command);

	eCommandState GetInternalCommandState(Command* command);
};

#include "../Qt/DockSceneGraph/PointerHolder.h"
DECLARE_POINTER_TYPE(Command *);


#endif // #ifndef __COMMAND_H__