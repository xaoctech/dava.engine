#ifndef __GUI_STATE_H__
#define __GUI_STATE_H__

#include "DAVAEngine.h"
#include "../Constants.h"

#define DECLARE_BOOL_PROPERTY(NAME)                                                     \
public:                                                                                 \
    void SetNeedUpdated##NAME(bool needUpdate) {needUpdated##NAME = needUpdate; };      \
    bool GetNeedUpdated##NAME() {return needUpdated##NAME; };                           \
private:                                                                                \
    bool needUpdated##NAME;                                                             \



class GUIState: public DAVA::Singleton<GUIState>
{
public:
    GUIState();
    virtual ~GUIState();
    
    DECLARE_BOOL_PROPERTY(FileMenu);
    DECLARE_BOOL_PROPERTY(ToolsMenu);
    DECLARE_BOOL_PROPERTY(Toolbar);
    DECLARE_BOOL_PROPERTY(Statusbar);
};

#endif // __GUI_STATE_H__
