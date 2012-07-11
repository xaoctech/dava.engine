#ifndef __COLOR_PICKER_H__
#define __COLOR_PICKER_H__

#include "DAVAEngine.h"
#include "ExtendedDialog.h"
#include "PropertyList.h"

using namespace DAVA;

class ColorDetailControl: public UIControl
{
public:    
    ColorDetailControl(const Rect &rect);
    virtual ~ColorDetailControl();
    
	virtual void DrawAfterChilds(const UIGeometricData &geometricData);
    virtual void Input(UIEvent *currentInput);
    
    const Color & GetColor();


protected:
    virtual void ColorSelected(const Vector2 &point) = 0;
    
    Sprite *colorMap; 
    Color selectedColor;

    Vector2 markerPoint;
};


class ColorSelectorControl: public ColorDetailControl
{
public:    
    ColorSelectorControl(const Rect &rect);

    void SetColor(const Color &color);
    
protected:
    virtual void ColorSelected(const Vector2 &point);
    
    void SetInitialColors();
    
    Color sections[6];
    
};


class ColorMapControl: public ColorDetailControl
{
public:    
    ColorMapControl(const Rect &rect);
    void SetColor(const Color &color);
    
protected:
    virtual void ColorSelected(const Vector2 &point);
    
    Color HSBToRgb(float32 s, float32 b);
    int32 RGBToH(const Color &color);
    int32 hue;
};


class ColorPickerDelegate;
class ColorPicker : public ExtendedDialog, public PropertyListDelegate
{
public:
    ColorPicker(ColorPickerDelegate *newDelegate);
    virtual ~ColorPicker();

    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    
    void SetColor(const Color & newColor);
    void Show();
    
protected:

    virtual const Rect DialogRect();

    void SetColor(const Color & newColor, bool updateColorMap, bool updateSelector);

    void OnOk(BaseObject * owner, void * userData, void * callerData);
    void OnCancel(BaseObject * owner, void * userData, void * callerData);
    
    void OnMapColorChanged(BaseObject * owner, void * userData, void * callerData);
    void OnSelectorColorChanged(BaseObject * owner, void * userData, void * callerData);

    void OnAlphaChanged(BaseObject * owner, void * userData, void * callerData);

    UISlider *alphaValue;

    UIControl *colorPreviewCurrent;
    UIControl *colorPreviewPrev;
    
    ColorPickerDelegate *delegate;
    
    ColorMapControl *colorMapControl;
    ColorSelectorControl *colorSelectorControl;

    PropertyList *colorList;

    Color prevColor;
    Color currentColor;
};



#endif // __SCENE_INFO_CONTROL_H__