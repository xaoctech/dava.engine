#include "HelpDialog.h"
#include "ControlsFactory.h"

#include "config.h"

HelpDialog::HelpDialog()
    :   ExtendedDialog()
{
    Rect rect = GetDialogRect();
    draggableDialog->SetRect(rect);
    
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH) / 2;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    closeButton = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &HelpDialog::OnCancel));
    draggableDialog->AddControl(closeButton);
    
    float32 y = 1;
	AddHelpText(L"F1/H - this help", y);
	AddHelpText(L"A W S D - fly camera", y);
	AddHelpText(L"1, 2, 3, 4 - set camera speed", y);
	AddHelpText(L"T - set camera to Top position", y);
	AddHelpText(L"Left mouse button - selection", y);
	AddHelpText(L"Right mouse button - camera angle", y);
	AddHelpText(L"Z - zoom to selection", y);	
	AddHelpText(L"BackSpace - remove selected object", y);
	AddHelpText(L"Esc - drop selection", y);	
	AddHelpText(L"Left mouse button (in selection) - object modification", y);
	AddHelpText(L"Drag with left mouse button + SHIFT (create copy of object)", y);
	AddHelpText(L"Middle mouse button (in selection) - move in camera plain", y);
	AddHelpText(L"Alt + Middle mouse button (in selection) rotate about selected objects", y);
	AddHelpText(L"Q, E, R (in selection) - change active modification mode (move, translate, scale)", y);
	AddHelpText(L"5, 6, 7 (in selection) - change active axis", y);
	AddHelpText(L"8 (in selection) - enumerate pairs of axis", y);
	AddHelpText(L"P (in selection) - place node on landscape", y);
    AddHelpText(L"Alt + 1...8: set SetForceLodLayer(0, 1, ... , 7)", y);
    AddHelpText(L"Alt + 0: set SetForceLodLayer(-1)", y);
    
    AddHelpText(L"Landscape Editor:", ++y);
	AddHelpText(L"Press & hold \"Spacebar\" to scroll area", y);
    
    AddHelpText(L"Scene Graph:", ++y);
    AddHelpText(L"Left mouse with Command/Ctrl key - change parent of node", y);
    AddHelpText(L"Right mouse with Shift key - change order of node", y);
    
	AddHelpText(L"version "EDITOR_VERSION, ++y);
}

HelpDialog::~HelpDialog()
{
    SafeRelease(closeButton);
}

void HelpDialog::Show()
{
    if(!GetParent())
    {
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


const Rect HelpDialog::GetDialogRect() const
{
    float32 width = 510.f;
    float32 height = 500.f;
    float32 x = (GetScreenRect().dx - width)/2.f;
    float32 y = (GetScreenRect().dy - height)/2.f;
    
    return Rect(x, y, width, height);
}

void HelpDialog::OnCancel(BaseObject *, void *, void *)
{
    Close();
}

#define V_OFFSET 30
#define H_OFFSET 10
void HelpDialog::AddHelpText(const WideString &txt, float32 & y)
{
	UIStaticText *text;
	text = new UIStaticText(Rect(H_OFFSET, 0, 500, y++ * V_OFFSET));
	text->SetFont(ControlsFactory::GetFont12());
	text->SetTextColor(ControlsFactory::GetColorLight());
	text->SetText(txt);
	text->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
	draggableDialog->AddControl(text);	
}

