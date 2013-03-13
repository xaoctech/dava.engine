#include "UISliderWithText.h"
#include "ControlsFactory.h"

REGISTER_CLASS(UISliderWithText);

UISliderWithText::UISliderWithText()
	: UISlider()
{
    sliderText = new UIStaticText(Rect(0, -40, 100, 40));
    sliderText->SetFont(ControlsFactory::GetFont12());
	sliderText->SetTextColor(ControlsFactory::GetColorLight());
    sliderText->SetAlign(ALIGN_HCENTER | ALIGN_BOTTOM);
    sliderText->SetInputEnabled(false);
    
    AddControl(sliderText);
    
    SetSliderText(oldValue = 0.f);
}

UISliderWithText::UISliderWithText(const Rect & rect)
:	UISlider(rect)
{
    sliderText = new UIStaticText(Rect(0, -40, 100, 40));
    sliderText->SetFont(ControlsFactory::GetFont12());
	sliderText->SetTextColor(ControlsFactory::GetColorLight());
    sliderText->SetAlign(ALIGN_HCENTER | ALIGN_BOTTOM);
    sliderText->SetInputEnabled(false);
    
    AddControl(sliderText);
    
    SetSliderText(oldValue = 0.f);
}

UISliderWithText::~UISliderWithText()
{
    SafeRelease(sliderText);
}

void UISliderWithText::Draw( const UIGeometricData &geometricData )
{
    UISlider::Draw(geometricData);
    
    if(oldValue != this->GetValue())
    {
        SetSliderText(oldValue = this->GetValue());
        
        //Draw text
        const Rect & tRect =  thumbButton->GetRect();
        Rect r = sliderText->GetRect();
        float32 x = tRect.x + (tRect.dx / 2.0f) - (r.dx / 2.0f);
        sliderText->SetRect(Rect(x, r.y, r.dx, r.dy));
    }
}

void UISliderWithText::SetSliderText(float32 value)
{
    sliderText->SetText(Format(L"%f", value));
}
