#ifndef __OUTPUT_CONTROL_H__
#define __OUTPUT_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class OutputControl: public UIControl, public UIListDelegate
{
    enum eConst
    {
        HISTORY_SIZE = 20,
        CELL_HEIGHT = 15,
    };
    
public:
    
    enum eMessageType
    {
        EMT_LOG = 0,
        EMT_WARNING,
        EMT_ERROR
    };
    
    
    struct LogMessage
    {
        WideString text;
        eMessageType type;
    };
    
public:
    OutputControl(const Rect & rect);
    virtual ~OutputControl();
    
    virtual void WillAppear();
    
    virtual int32 ElementsCount(UIList * list);
    virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);
    
    void Log(const WideString &message);
    void Warning(const WideString &message);
    void Error(const WideString &message);
    
    void Clear();
    
protected:

    void ReleaseList();
    
    void AddMessageToList(eMessageType type, const WideString &message);
    
    UIList *messageList;
    
    Vector<LogMessage *> messages;

};



#endif // __OUTPUT_CONTROL_H__