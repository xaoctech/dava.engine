#ifndef __UI_SLIDER_WITHTEXT_H__
#define __UI_SLIDER_WITHTEXT_H__

#include "DAVAEngine.h"


using namespace DAVA;


class UISliderWithText : public UISlider
{
public:

	UISliderWithText();
	UISliderWithText(const Rect & rect);
    
    virtual ~UISliderWithText();

	virtual void Draw(const UIGeometricData &geometricData);

private:

    void SetSliderText(float32 value);
    
    UIStaticText *sliderText;
    float32 oldValue;
};

#endif //#ifndef __UI_SLIDER_WITHTEXT_H__

