#include "ColorControl.h"
#include "ControlsFactory.h"

#include "EditorSettings.h"

ColorControl::ColorControl(const Rect & rect, ColorControlDelegate *newDelegate)
    :   UIControl(rect)
{
    ControlsFactory::CustomizeDialog(this);
    
    delegate = newDelegate;
    
    Rect propertyRect(0, 0, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT);
    colorProperties = new PropertyList(propertyRect, NULL);
    AddControl(colorProperties);
    
	SetupProperties();
    
    Rect buttonRect(0, rect.dy - ControlsFactory::BUTTON_HEIGHT, rect.dx / 2, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnCancel = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ColorControl::OnCancel));
    AddControl(btnCancel);
    SafeRelease(btnCancel);

    buttonRect.x = rect.dx - buttonRect.dx;
    UIButton *btnCreate = ControlsFactory::CreateButton(buttonRect, LocalizedString(L"dialog.set"));
    btnCreate->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ColorControl::OnSet));
    AddControl(btnCreate);
    SafeRelease(btnCreate);
}
    
ColorControl::~ColorControl()
{
    delegate = 0;
    SafeRelease(colorProperties);
}

void ColorControl::SetupProperties()
{
	colorProperties->AddSubsection(String("Color Settings"));

	colorProperties->AddColorProperty("property.material.ambientcolor");
	colorProperties->SetColorPropertyValue("property.material.ambientcolor", EditorSettings::Instance()->GetMaterialAmbientColor());

	colorProperties->AddColorProperty("property.material.diffusecolor");
	colorProperties->SetColorPropertyValue("property.material.diffusecolor", EditorSettings::Instance()->GetMaterialDiffuseColor());

	colorProperties->AddColorProperty("property.material.specularcolor");
	colorProperties->SetColorPropertyValue("property.material.specularcolor", EditorSettings::Instance()->GetMaterialSpecularColor());
}

void ColorControl::WillAppear()
{
}

void ColorControl::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}

void ColorControl::OnSet(BaseObject * object, void * userData, void * callerData)
{
    if(delegate && colorProperties)
    {
		Color ambient = colorProperties->GetColorPropertyValue(String("property.material.ambientcolor"));
		Color diffuse = colorProperties->GetColorPropertyValue(String("property.material.diffusecolor"));
		Color specular = colorProperties->GetColorPropertyValue(String("property.material.specularcolor"));

		EditorSettings::Instance()->SetMaterialsColor(ambient, diffuse, specular);
		EditorSettings::Instance()->Save();

        delegate->SetupColor(ambient, diffuse, specular);
    }
    
    OnCancel(NULL, NULL, NULL);
}

