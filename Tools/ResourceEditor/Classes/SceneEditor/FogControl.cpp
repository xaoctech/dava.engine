#include "FogControl.h"
#include "ControlsFactory.h"

FogControl::FogControl(const Rect & rect, FogControlDelegate *newDelegate)
    :   UIControl(rect)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    
    Rect propertyRect(0, 0, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT);
    fogProperties = new PropertyList(propertyRect, NULL);
    AddControl(fogProperties);
    
    ControlsFactory::AddFogSubsection(fogProperties, true, 0.006f, Color::White());
    
    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &FogControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.set"));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &FogControl::OnSet));
    AddControl(btnCreate);
    SafeRelease(btnCreate);
}
    
FogControl::~FogControl()
{
    delegate = 0;
    SafeRelease(fogProperties);
}

void FogControl::WillAppear()
{
}

void FogControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void FogControl::OnSet(BaseObject * object, void * userData, void * callerData)
{
    if(delegate && fogProperties)
    {
        delegate->SetupFog(fogProperties->GetBoolPropertyValue(String("property.material.fogenabled")), 
                           fogProperties->GetFloatPropertyValue(String("property.material.dencity")), 
                           fogProperties->GetColorPropertyValue(String("property.material.fogcolor")));
    }
    
    OnCancel(NULL, NULL, NULL);
}

