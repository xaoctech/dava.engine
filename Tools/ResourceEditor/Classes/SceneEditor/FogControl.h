#ifndef __FOG_CONTROL_H__
#define __FOG_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

using namespace DAVA;

class FogControlDelegate
{
public:
    virtual void SetupFog(bool enabled, float32 dencity, const Color &newColor) = 0;
};

class FogControl: public UIControl
{
    
public:
    FogControl(const Rect & rect, FogControlDelegate *newDelegate);
    virtual ~FogControl();
    
    virtual void WillAppear();
    
protected:

    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnSet(BaseObject * object, void * userData, void * callerData);
    
    FogControlDelegate *delegate;
    PropertyList *fogProperties;
};



#endif // __FOG_CONTROL_H__