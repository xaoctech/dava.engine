#ifndef __HINT_MANAGER_H__
#define __HINT_MANAGER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class HintControl: public UIControl
{
public:
    
    HintControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    virtual ~HintControl();
    
    void SetText(const WideString &hintMessage);
    
protected:
    
    UIStaticText *hintText;
};


class HintManager: public Singleton<HintManager>
{
    enum eConst
    {
        NOTIFICATION_TIME = 3
    };
    
public:
    HintManager();
    virtual ~HintManager();

    void ShowHint(const WideString &hintMessage, const Rect &controlRectAbsolute);
    
protected:

    void OnAlphaAnimationDone(BaseObject * owner, void * userData, void * callerData);

    Vector<HintControl *> hints;

};



#endif // __HINT_MANAGER_H__