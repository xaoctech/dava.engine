//
//  BackgroundPropertiesSection.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "BackgroundPropertiesSection.h"

#include "ValueProperty.h"

using namespace DAVA;

BackgroundPropertiesSection::BackgroundPropertiesSection(UIControl *control, int bgNum) : control(NULL), bgNum(bgNum), isContentHidden(false)
{
    this->control = SafeRetain(control);
}

BackgroundPropertiesSection::~BackgroundPropertiesSection()
{
    SafeRelease(control);
}

PropertiesSection *BackgroundPropertiesSection::CopyAndApplyForNewControl(UIControl *newControl)
{
    BackgroundPropertiesSection *section = new BackgroundPropertiesSection(newControl, bgNum);
    if (control->GetBackgroundComponent(bgNum) != NULL && newControl->GetBackgroundComponent(bgNum) == NULL)
    {
        UIControlBackground *bg = newControl->CreateBackgroundComponent(bgNum);
        newControl->SetBackgroundComponent(bgNum, bg);
        SafeRelease(bg);
    }

    UIControlBackground *bg = newControl->GetBackgroundComponent(bgNum);
    if (bg)
    {
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            const InspMember *member = (*it)->GetMember();
            member->SetValue(bg, (*it)->GetValue());
//            ValueProperty *prop = new ValueProperty(bg, member);
//            section->AddProperty(prop);
//            SafeRelease(prop);
        }
    }
    
    return section;
}

DAVA::String BackgroundPropertiesSection::GetName() const
{
    return control->GetBackgroundComponentName(bgNum);
}

int BackgroundPropertiesSection::GetCount() const
{
    return isContentHidden ? 0 : PropertiesSection::GetCount();
}

void BackgroundPropertiesSection::HideContent()
{
    isContentHidden = true;
}
