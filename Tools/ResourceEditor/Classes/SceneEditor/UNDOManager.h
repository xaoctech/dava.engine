#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class UNDOAction: public BaseObject
{
public:
    enum eActionType
    {
        ACTION_NONE = -1,
        ACTION_HEIGHTMAP = 0
    };
    
    UNDOAction();

    eActionType type;
    int32 ID;
    String filePathname;
};


class UNDOManager: public Singleton<UNDOManager>
{    
    enum eConst
    {
        UNDO_HISTORY_SIZE = 50
    };
    
    
public:
 
    UNDOManager();
	virtual ~UNDOManager();

    void SaveHightmap(Heightmap *heightmap);
    void UndoHeightmap(Heightmap *heightmap);
    
    UNDOAction::eActionType GetLastUNDOAction();

protected:

    String TimeString();
    
    void CheckHistoryLength();
    
    List<UNDOAction *>actionsHistory;
    int32 actionCounter;
    
};



#endif // __UNDO_MANAGER_H__