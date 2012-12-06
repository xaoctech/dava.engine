#ifndef __COLOR_CONTROL_H__
#define __COLOR_CONTROL_H__

#include "DAVAEngine.h"
#include "PropertyList.h"

using namespace DAVA;

class ColorControlDelegate
{
public:
    virtual void SetupColor(const Color &ambient, const Color &diffuse, const Color &specular) = 0;
};

class ColorControl: public UIControl
{
    
public:
    ColorControl(const Rect & rect, ColorControlDelegate *newDelegate);
    virtual ~ColorControl();
    
    virtual void WillAppear();
    
protected:

	void SetupProperties();


    void OnCancel(BaseObject * object, void * userData, void * callerData);
    void OnSet(BaseObject * object, void * userData, void * callerData);
    
    ColorControlDelegate *delegate;
    PropertyList *colorProperties;
};



#endif // __COLOR_CONTROL_H__