#include "ControlsFactory.h"

UIButton * ControlsFactory::CreateButton(Rect r, const WideString &buttonText)
{
    UIButton *btn = new UIButton(r);
    CustomizeButton(btn, buttonText);
    return btn;
}

void ControlsFactory::CustomizeButton(UIButton *btn, const WideString &buttonText)
{
    Font *font = CreateFontLight();
    
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_DISABLED, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
    
    btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    btn->GetStateBackground(UIControl::STATE_DISABLED)->SetColor(Color(0.2f, 0.2f, 0.2f, 0.2f));
    btn->GetStateBackground(UIControl::STATE_SELECTED)->SetColor(Color(0.0f, 0.0f, 1.0f, 0.2f));
    
    btn->SetStateFont(UIControl::STATE_PRESSED_INSIDE, font);
    btn->SetStateFont(UIControl::STATE_DISABLED, font);
    btn->SetStateFont(UIControl::STATE_NORMAL, font);
    btn->SetStateFont(UIControl::STATE_SELECTED, font);
    
    btn->SetStateText(UIControl::STATE_PRESSED_INSIDE, buttonText);
    btn->SetStateText(UIControl::STATE_DISABLED, buttonText);
    btn->SetStateText(UIControl::STATE_NORMAL, buttonText);
    btn->SetStateText(UIControl::STATE_SELECTED, buttonText);
    
    SafeRelease(font);
}

UIButton * ControlsFactory::CreateCloseWindowButton(Rect r)
{
    UIButton *btn = new UIButton(r);
    CustomizeCloseWindowButton(btn);
    return btn;
}

void ControlsFactory::CustomizeCloseWindowButton(UIButton *btn)
{
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    
    btn->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.7f, 0.0, 0.0, 1.f));
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.25f, 0.0, 0.0, 1.f));
}


Font * ControlsFactory::CreateFontLight()
{
    FTFont *font = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    CustomizeFontLight(font);
    return font;
}

void ControlsFactory::CustomizeFontLight(Font *font)
{
    font->SetSize(12);
    font->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
}

Font * ControlsFactory::CreateFontDark()
{
    FTFont *font = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    CustomizeFontDark(font);
    return font;
}

void ControlsFactory::CustomizeFontDark(Font *font)
{
    font->SetSize(12);
    font->SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
}

void ControlsFactory::CustomizeScreenBack(UIControl *screen)
{
    screen->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    screen->GetBackground()->SetColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
}

UIControl * ControlsFactory::CreateLine(Rect r)
{
    UIControl * lineControl = new UIControl(r); 
    lineControl->GetBackground()->color = Color(0.8f, 0.8f, 0.8f, 1.0f);
    lineControl->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    return lineControl;
}

void ControlsFactory::CusomizeBottomLevelControl(UIControl *c)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
}

void ControlsFactory::CusomizeTopLevelControl(UIControl *c)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
}

void ControlsFactory::CusomizeListControl(UIControl *c)
{
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    c->GetBackground()->SetColor(Color(0.92f, 0.92f, 0.92f, 1.0f));
}

UIControl * ControlsFactory::CreatePanelControl(Rect r)
{
    UIControl *ctrl = new UIControl(r);
    CustomizePanelControl(ctrl);
    return ctrl;
}

void ControlsFactory::CustomizePanelControl(UIControl *c)
{
    c->GetBackground()->color = Color(0.4f, 0.4f, 0.4f, 1.0f);
    c->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
}

void ControlsFactory::CustomizeExpandButton(UIButton *btn)
{
    Color color(0.1f, 0.5f, 0.05f, 1.0f);
    btn->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    btn->SetStateDrawType(UIControl::STATE_HOVER, UIControlBackground::DRAW_FILL);
    btn->GetStateBackground(UIControl::STATE_NORMAL)->color = color;
    btn->GetStateBackground(UIControl::STATE_HOVER)->color = color + 0.1f;
    btn->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->color = color + 0.3f;
}

void ControlsFactory::CustomizeListCell(UIListCell *c)
{
    Font *font = CreateFontDark();
    
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    c->SetStateFont(UIControl::STATE_SELECTED, font);

    
//    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
//    c->GetStateBackground(UIControl::STATE_NORMAL)->color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);
    
    SafeRelease(font);
}

void ControlsFactory::CustomizeHierarhyCell(UIHierarchyCell *c)
{
    Font *font = CreateFontDark();
    
    c->text->SetFont(font);
    c->text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);
    
//    c->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->SetStateDrawType(UIControl::STATE_SELECTED, UIControlBackground::DRAW_FILL);
//    c->GetStateBackground(UIControl::STATE_NORMAL)->color = Color(1.0f, 1.0f, 1.0f, 1.0f);
    c->GetStateBackground(UIControl::STATE_SELECTED)->color = Color(1.0f, 0.8f, 0.8f, 1.0f);
    
    SafeRelease(font);
}


