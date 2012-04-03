#include "HintManager.h"
#include "ControlsFactory.h"


#pragma mark  --HintControl
HintControl::HintControl(const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    ControlsFactory::CusomizeBottomLevelControl(this);

    hintText = new UIStaticText();
    hintText->SetFont(ControlsFactory::GetFontDark());
    hintText->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    hintText->SetText(L"");
    AddControl(hintText);
}

HintControl::~HintControl()
{
    SafeRelease(hintText);
}

void HintControl::SetText(const WideString &hintMessage)
{
    Size2i requestedSize = hintText->GetFont()->GetStringSize(hintMessage);
    Vector2 controlSize(requestedSize.dx + 20, requestedSize.dy + 10);
    hintText->SetRect(Rect(0, 0, controlSize.dx, controlSize.dy));
    this->SetSize(controlSize);

    hintText->SetText(hintMessage);
}

#pragma mark  --HintManager
HintManager::HintManager()
{
}
    
HintManager::~HintManager()
{
    for(int32 h = 0; h < hints.size(); ++h)
    {
        if(hints[h] && hints[h]->GetParent())
        {
            hints[h]->GetParent()->RemoveControl(hints[h]);
        }
        SafeRelease(hints[h]);
    }
    hints.clear();
}

void HintManager::ShowHint(const WideString &hintMessage, const DAVA::Rect &controlRectAbsolute)
{
    if(0 != hintMessage.length())
    {
        //Add control
        HintControl *hintControl = new HintControl();
        hintControl->SetText(hintMessage);
        
        Rect screenRect = UIScreenManager::Instance()->GetScreen()->GetRect(true);
        
        Vector2 requestedSize = hintControl->GetSize();
        if(requestedSize.dx < controlRectAbsolute.GetSize().dx)
        {
            Vector2 pos(controlRectAbsolute.x - screenRect.x, controlRectAbsolute.y - screenRect.y + controlRectAbsolute.dy);
            hintControl->SetPosition(pos);
        }
        else if(controlRectAbsolute.x - screenRect.x < requestedSize.dx)
        {
            Vector2 pos(controlRectAbsolute.x - screenRect.x, controlRectAbsolute.y - screenRect.y + controlRectAbsolute.dy);
            hintControl->SetPosition(pos);
        }
        else 
        {
            Vector2 pos(controlRectAbsolute.x - screenRect.x + controlRectAbsolute.dx - requestedSize.dx, controlRectAbsolute.y - screenRect.y + controlRectAbsolute.dy);
            hintControl->SetPosition(pos);
        }
        
        ControlsFactory::AddBorder(hintControl);
        UIScreenManager::Instance()->GetScreen()->AddControl(hintControl);
        
        Animation *hintAlphaAnimation = hintControl->ColorAnimation( Color::Transparent(), NOTIFICATION_TIME, 
                                                                     Interpolation::EASY_IN, 2);
        hintAlphaAnimation->AddEvent(Animation::EVENT_ANIMATION_END, 
                                    Message(this, &HintManager::OnAlphaAnimationDone, hintControl));

        
        
        
        
        hints.push_back(hintControl);
    }
}

void HintManager::OnAlphaAnimationDone(BaseObject * owner, void * userData, void * callerData)
{
    UIControl *hintControl = (UIControl *)userData;

    for(Vector<HintControl*>::iterator it = hints.begin(); it != hints.end(); ++it)
    {
        if((*it) == hintControl && hintControl)
        {
            if(hintControl->GetParent())
            {
                hintControl->GetParent()->RemoveControl(hintControl);
            }
            
            SafeRelease(hintControl);
            hints.erase(it);
            break;
        }
    }
}


