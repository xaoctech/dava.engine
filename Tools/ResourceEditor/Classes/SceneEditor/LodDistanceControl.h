#ifndef __LODDISTANCE_CONTROL_H__
#define __LODDISTANCE_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class LodDistanceControl;
class LodDistanceControlDelegate
{
public:
    
    virtual void DistanceChanged(LodDistanceControl *forControl, int32 index, float32 value) = 0;
};

class LodDistanceControl : public UIControl, public UITextFieldDelegate
{
public:

	LodDistanceControl(LodDistanceControlDelegate *newDelegate, const Rect &rect, bool rectInAbsoluteCoordinates = false);
    virtual ~LodDistanceControl();

    virtual void WillDisappear();
    
    void SetDistances(float32 *newDistances, int32 newCount);
    
    virtual void Input(UIEvent *currentInput);
    
    //UITextFieldDelegate
    virtual void TextFieldShouldReturn(UITextField * textField);
    virtual void TextFieldShouldCancel(UITextField * textField);
    virtual void TextFieldLostFocus(UITextField * textField);
	virtual bool TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, 
                                     int32 replacementLength, const WideString & replacementString);
    
private:

    void ReleaseControls();
    
    UIControl **zones;
    UIControl **sliders;
    
    int32 count;
    float32 distances[LodNode::MAX_LOD_DISTANCE];
    float32 maxDistance;

    LodDistanceControlDelegate *delegate;
    
    int32 mainTouch;
    float32 oldPos;
	float32 newPos;
    
    UIControl *activeSlider;
    void UpdateSliderPos();
    
    Vector2 leftEdge;
    Vector2 rightEdge;
    
    UIControl *leftZone;
    UIControl *rightZone;
    
    UIStaticText *distanceText[LodNode::MAX_LOD_LAYERS];
    UITextField *distanceTextValues[LodNode::MAX_LOD_LAYERS];
    
    int32 activeLodIndex;
};


#endif //#ifndef __LODDISTANCE_CONTROL_H__
