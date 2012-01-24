#ifndef __COLOR_PICKER_DELEGATE_H__
#define __COLOR_PICKER_DELEGATE_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ColorPickerDelegate
{
public:
    
    virtual void ColorPickerDone(const Color &newColor) = 0;
};


#endif // __COLOR_PICKER_DELEGATE_H__