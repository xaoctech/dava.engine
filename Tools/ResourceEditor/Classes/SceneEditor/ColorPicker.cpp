#include "ColorPicker.h"
#include "ControlsFactory.h"
#include "ColorPickerDelegate.h"

//*************  ColorDetailControl  *************
ColorDetailControl::ColorDetailControl(const Rect &rect)
:   UIControl(rect)
{
    colorMap = Sprite::CreateAsRenderTarget(rect.dx, rect.dy, Texture::FORMAT_RGBA8888);
    SetSprite(colorMap, 0);
    
    selectedColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

ColorDetailControl::~ColorDetailControl()
{
    SafeRelease(colorMap);
}

void ColorDetailControl::Input(UIEvent *currentInput)
{
	if ((UIEvent::BUTTON_1 == currentInput->tid) && (UIEvent::PHASE_MOVE != currentInput->phase))
	{
        Vector2 touchPoint = currentInput->point - GetPosition(true);
        touchPoint.x = Min(touchPoint.x, GetRect().dx);
        touchPoint.y = Min(touchPoint.y, GetRect().dy);
        
        ColorSelected(touchPoint);
	}
}

const Color & ColorDetailControl::GetColor()
{
    return selectedColor;
}


//*************  ColorSelectorControl  *************
ColorSelectorControl::ColorSelectorControl(const Rect &rect)
    :   ColorDetailControl(rect)
{
    RenderManager::Instance()->SetRenderTarget(colorMap);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 1.f));
    RenderHelper::Instance()->FillRect(Rect(0,0, rect.dx, rect.dy));

    SetInitialColors();

    int32 sectionHeight = rect.dy / 6.f;
    
    float32 colorDelta = 1.0f / (sectionHeight);
    for(int32 dy = 0; dy < sectionHeight; ++dy)
    {
        for(int32 iSection = 0; iSection < 6; ++iSection)
        {
            RenderManager::Instance()->SetColor(sections[iSection]);
            RenderHelper::Instance()->DrawLine(Vector2(0, iSection*sectionHeight + dy), 
                                               Vector2(rect.dx, iSection*sectionHeight + dy));
        }
        
        sections[0].b += colorDelta;
        sections[1].r -= colorDelta;
        sections[2].g += colorDelta;
        sections[3].b -= colorDelta;
        sections[4].r += colorDelta;
        sections[5].g -= colorDelta;
    }

    RenderManager::Instance()->ResetColor();
    RenderManager::Instance()->RestoreRenderTarget();

    SetInitialColors();
}

void ColorSelectorControl::SetInitialColors()
{
    sections[0] = Color(1.0f, 0.f, 0.f, 1.0f);
    sections[1] = Color(1.0f, 0.f, 1.f, 1.0f);
    sections[2] = Color(0.0f, 0.f, 1.f, 1.0f);
    sections[3] = Color(0.0f, 1.f, 1.f, 1.0f);
    sections[4] = Color(0.0f, 1.f, 0.f, 1.0f);
    sections[5] = Color(1.0f, 1.f, 0.f, 1.0f);
}

void ColorSelectorControl::ColorSelected(const Vector2 &point)
{
    int32 sectionHeight = GetRect().dy / 6.f;
    float32 colorDelta = 1.0f / (sectionHeight);

    int32 sectionId = point.y / sectionHeight;
    float32 sectionDelta = (point.y - (sectionHeight * sectionId));
    
    selectedColor = sections[sectionId];
    switch (sectionId)
    {
        case 0:
            selectedColor.b += colorDelta * sectionDelta;
            break;
        case 1:
            selectedColor.r -= colorDelta * sectionDelta;
            break;
        case 2:
            selectedColor.g += colorDelta * sectionDelta;
            break;
        case 3:
            selectedColor.b -= colorDelta * sectionDelta;
            break;
        case 4:
            selectedColor.r += colorDelta * sectionDelta;
            break;
        case 5:
            selectedColor.g -= colorDelta * sectionDelta;
            break;

        default:
            break;
    }
    
    DVASSERT(selectedColor.r >= 0);
    DVASSERT(selectedColor.g >= 0);
    DVASSERT(selectedColor.b >= 0);

    this->PerformEvent(UIControl::EVENT_VALUE_CHANGED);
}


//*************  ColorMapControl  *************
ColorMapControl::ColorMapControl(const Rect &rect)
    :   ColorDetailControl(rect)
{
    hue = 0;
}

void ColorMapControl::SetColor(const Color &color)
{
    RenderManager::Instance()->SetRenderTarget(colorMap);
    RenderManager::Instance()->SetColor(Color(0.f, 0.f, 0.f, 1.f));
    RenderHelper::Instance()->FillRect(Rect(0,0, GetRect().dx, GetRect().dy));
    
    hue = RGBToH(color);
    
    float32 dx = GetRect().dx / 101.f;
    float32 dy = GetRect().dy / 101.f;
    float32 y = 0.f;
    for(int32 brightness = 100; brightness >= 0; --brightness)
    {
        float32 x = 0.f;
        for(int32 saturation = 0; saturation <= 100; ++saturation)
        {
            Color cellColor = HSBToRgb(saturation / 100.f, brightness / 100.f);
            RenderManager::Instance()->SetColor(cellColor);
            RenderHelper::Instance()->FillRect(Rect(x, y, dx, dy));
            x += dx;
        }
        
        y += dy;
    }
    
    RenderManager::Instance()->RestoreRenderTarget();
}

Color ColorMapControl::HSBToRgb(float32 s, float32 b)
{
    Color c;
    
    if (0.f == s) 
    {
        c.r = c.g = c.b = b;
    }
    else 
    {
        float32 h = hue/ 60.f;
        int32 i = floorf(h);
        float32 f = h - i;
        float32 p = b * (1.0f - s);
        float32 q = b * (1.0f - s * f);
        float32 t = b * (1.0f - (s * (1.0f - f)));

        switch (i) 
        {
            case 0:
                c.r = b;
                c.g = t;
                c.b = p;
                break;
                
            case 1:
                c.r = q;
                c.g = b;
                c.b = p;
                break;
                
            case 2:
                c.r = p;
                c.g = b;
                c.b = t;
                break;
                
            case 3:
                c.r = p;
                c.g = q;
                c.b = b;
                break;
                
            case 4:
                c.r = t;
                c.g = p;
                c.b = b;
                break;
                
            case 5:
                c.r = b;
                c.g = p;
                c.b = q;
                break;
        }
    }
    
    c.a = 1.0f;
    return c;
}

int32 ColorMapControl::RGBToH(const DAVA::Color &color)
{
    //http://ru.wikipedia.org/wiki/HSV_(цветовая_модель)
    float32 min = Min(Min(color.r, color.g), color.b);
    float32 max = Max(Max(color.r, color.g), color.b);
    
    if(max != min)
    {
        if((max == color.r) && (color.g >= color.b))
        {
            return (60 * (color.g - color.b) / (max - min));
        }
        
        if((max == color.r) && (color.g < color.b))
        {
            return (60 * (color.g - color.b) / (max - min) + 360);
        }
        
        if(max == color.g)
        {
            return (60 * (color.b - color.r) / (max - min) + 120);
        }
        
        if(max == color.b)
        {
            return (60 * (color.r - color.g) / (max - min) + 240);
        }
    }
    
    return 0;
}
    
void ColorMapControl::ColorSelected(const Vector2 &point)
{
    float32 b = 1.f - (point.y/GetRect().dy);
    float32 s = point.x / GetRect().dx;
    
    selectedColor = HSBToRgb(s, b);
    this->PerformEvent(UIControl::EVENT_VALUE_CHANGED);
}


//*************  ColorPicker  *************
ColorPicker::ColorPicker(ColorPickerDelegate *newDelegate)
    :   ExtendedDialog()
    ,   delegate(newDelegate)
{
    ControlsFactory::CustomizeDialog(this);
    
    Rect rect = DialogRect();
    
    float32 mapWidth = rect.dx / 2 - (ControlsFactory::OFFSET * 2);
    float32 mapHeight = rect.dy - (ControlsFactory::OFFSET * 3) - ControlsFactory::BUTTON_HEIGHT * 2;

    colorMapControl = new ColorMapControl(Rect(ControlsFactory::OFFSET, ControlsFactory::OFFSET, mapWidth, mapHeight));
    colorMapControl->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &ColorPicker::OnMapColorChanged));
    draggableDialog->AddControl(colorMapControl);
    
    alphaValue = new UISlider(Rect(colorMapControl->GetRect().x, rect.dy - ControlsFactory::BUTTON_HEIGHT * 2 - ControlsFactory::OFFSET, 
                                   colorMapControl->GetRect().dx, ControlsFactory::BUTTON_HEIGHT));
    alphaValue->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &ColorPicker::OnAlphaChanged));
    alphaValue->SetMinMaxValue(0.f, 1.0f);
    alphaValue->SetValue(1.0f);
    alphaValue->SetMinSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 1);
    alphaValue->SetMinDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    alphaValue->SetMinLeftRightStretchCap(5);
    alphaValue->SetMaxSprite("~res:/Gfx/LandscapeEditor/Tools/polzunok", 0);
    alphaValue->SetMaxDrawType(UIControlBackground::DRAW_STRETCH_HORIZONTAL);
    alphaValue->SetMaxLeftRightStretchCap(5);
    alphaValue->SetThumbSprite("~res:/Gfx/LandscapeEditor/Tools/polzunokCenter", 0);
    draggableDialog->AddControl(alphaValue);

    float32 selectorWidth = rect.dx / 4 - (ControlsFactory::OFFSET * 2);
    colorSelectorControl = new ColorSelectorControl(Rect(rect.dx / 2 + ControlsFactory::OFFSET, 
                                                         ControlsFactory::OFFSET, selectorWidth, mapHeight));
    colorSelectorControl->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &ColorPicker::OnSelectorColorChanged));
    draggableDialog->AddControl(colorSelectorControl);
    
    colorPreviewCurrent = new UIControl(Rect(rect.dx *3 / 4 + ControlsFactory::OFFSET, ControlsFactory::OFFSET, selectorWidth, selectorWidth/2));
    colorPreviewCurrent->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    draggableDialog->AddControl(colorPreviewCurrent);

    Rect prevRect = colorPreviewCurrent->GetRect();
    prevRect.y += prevRect.dy;
    colorPreviewPrev = new UIControl(Rect(prevRect));
    colorPreviewPrev->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    draggableDialog->AddControl(colorPreviewPrev);
    
    colorList = new PropertyList(Rect(prevRect.x, prevRect.y + prevRect.dy + ControlsFactory::OFFSET, prevRect.dx, 80), this);
    ControlsFactory::CusomizeTransparentControl(colorList, 0.0f);
    colorList->AddIntProperty("colorpicker.r", PropertyList::PROPERTY_IS_EDITABLE);
    colorList->AddIntProperty("colorpicker.g", PropertyList::PROPERTY_IS_EDITABLE);
    colorList->AddIntProperty("colorpicker.b", PropertyList::PROPERTY_IS_EDITABLE);
    colorList->AddIntProperty("colorpicker.a", PropertyList::PROPERTY_IS_EDITABLE);
    draggableDialog->AddControl(colorList);
    
    float32 buttonX = rect.dx /2 - ControlsFactory::BUTTON_WIDTH;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ColorPicker::OnCancel));
    draggableDialog->AddControl(btnCancel);
    SafeRelease(btnCancel);
    
    buttonX += ControlsFactory::BUTTON_WIDTH;
    UIButton *btnOk = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.ok"));
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ColorPicker::OnOk));
    draggableDialog->AddControl(btnOk);
    SafeRelease(btnOk);
    
    SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
}

ColorPicker::~ColorPicker()
{
    SafeRelease(alphaValue);
    SafeRelease(colorPreviewCurrent);
    SafeRelease(colorPreviewPrev);
    SafeRelease(colorMapControl);
    SafeRelease(colorSelectorControl);
    SafeRelease(colorList);
}


void ColorPicker::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
    currentColor = Color(
                colorList->GetIntPropertyValue("colorpicker.r") / 255.f,
                colorList->GetIntPropertyValue("colorpicker.g") / 255.f,
                colorList->GetIntPropertyValue("colorpicker.b") / 255.f,
                colorList->GetIntPropertyValue("colorpicker.a") / 255.f
                );

    SetColor(currentColor, true);
}

void ColorPicker::OnOk(BaseObject * owner, void * userData, void * callerData)
{
    Color color(
                colorList->GetIntPropertyValue("colorpicker.r") / 255.f,
                colorList->GetIntPropertyValue("colorpicker.g") / 255.f,
                colorList->GetIntPropertyValue("colorpicker.b") / 255.f,
                colorList->GetIntPropertyValue("colorpicker.a") / 255.f);

    if(delegate)
    {
        delegate->ColorPickerDone(color);
    }
    
    Close();
}

void ColorPicker::OnCancel(BaseObject * owner, void * userData, void * callerData)
{
    Close();
}

void ColorPicker::OnMapColorChanged(BaseObject * owner, void * userData, void * callerData)
{
    Color color = colorMapControl->GetColor();
    SetColor(color, false);
}

void ColorPicker::OnSelectorColorChanged(BaseObject * owner, void * userData, void * callerData)
{
    Color color = colorSelectorControl->GetColor();
    SetColor(color, true);
}

void ColorPicker::SetColor(const DAVA::Color &newColor)
{
    prevColor = newColor;
    currentColor = newColor;
    
    colorPreviewPrev->GetBackground()->SetColor(Color(prevColor.r, prevColor.g, prevColor.b, 1.0f));
    
    SetColor(newColor, true);
}

void ColorPicker::SetColor(const Color & newColor, bool updateColorMap)
{
    currentColor.r = newColor.r;
    currentColor.g = newColor.g;
    currentColor.b = newColor.b;

    colorPreviewCurrent->GetBackground()->SetColor(Color(currentColor.r, currentColor.g, currentColor.b, 1.0f));
    if(updateColorMap)
    {
        colorMapControl->SetColor(currentColor);   
    }
    
    colorList->SetIntPropertyValue("colorpicker.r", currentColor.r * 255);
    colorList->SetIntPropertyValue("colorpicker.g", currentColor.g * 255);
    colorList->SetIntPropertyValue("colorpicker.b", currentColor.b * 255);
    colorList->SetIntPropertyValue("colorpicker.a", currentColor.a * 255);
    
    alphaValue->SetValue(currentColor.a);
}


void ColorPicker::Close()
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

//void ColorPicker::DidAppear()
//{
//    //Set cells background
//    List<UIControl*> cells = colorList->GetVisibleCells();
//    for(List<UIControl*>::iterator it = cells.begin(); it != cells.end(); ++it)
//    {
//        UIControl *cell = (*it);
//        ControlsFactory::CusomizeTransparentControl(cell, 0.0f);
//    }
//}

void ColorPicker::OnAlphaChanged(BaseObject * owner, void * userData, void * callerData)
{
    currentColor.a = alphaValue->GetValue();
    SetColor(currentColor, true);
}

void ColorPicker::Show()
{
    if(!GetParent())
    {
        UIScreenManager::Instance()->GetScreen()->AddControl(this);
    }
}
