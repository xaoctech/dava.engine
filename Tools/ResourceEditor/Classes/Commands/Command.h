#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "DAVAEngine.h"

class CommandsManager;
class Command: public DAVA::BaseObject
{
    friend class CommandsManager;
    
public:
    
    enum eCommandType
    {
        COMMAND_WITHOUT_UNDO_EFFECT = 0,    // Command has no effect onto undo-redo queue
        COMMAND_CLEAR_UNDO_QUEUE,           // Command reset undo queue
        COMMAND_UNDO_REDO                   // You cat undo & redo this command
    };
    
public:	
	Command(eCommandType _type);
	virtual ~Command();

protected:	
    
    virtual void Execute() = 0;
    virtual void Cancel() {};
    
    inline eCommandType Type() const {return commandType; };
	
protected:
    
    eCommandType commandType;
};



#endif // #ifndef __COMMAND_H__