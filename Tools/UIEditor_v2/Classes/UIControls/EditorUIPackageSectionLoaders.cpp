//
//  EditorUIPackageSectionLoaders.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 8.10.14.
//
//

#include "EditorUIPackageSectionLoaders.h"

#include "ControlPropertiesSection.h"
#include "ValueProperty.h"
#include "BackgroundPropertiesSection.h"
#include "InternalControlPropertiesSection.h"
#include "LocalizedTextValueProperty.h"
#include "UIEditorComponent.h"

using namespace DAVA;

EditorUIPackageControlSectionLoader::EditorUIPackageControlSectionLoader(UIControl *control, const String &name) : UIPackageControlSectionLoader(control, name)
{
    section = new ControlPropertiesSection(name);
}

void EditorUIPackageControlSectionLoader::SetProperty(const InspMember *member, const DAVA::VariantType &value)
{
    if ((member->Flags() & I_EDIT) != 0)
    {
        ValueProperty *property = NULL;
        if (String(member->Name()) == "text")
            property = new LocalizedTextValueProperty(GetBaseObject(), member);
        else
            property = new ValueProperty(GetBaseObject(), member);
        if (value.GetType() != VariantType::TYPE_NONE)
            property->SetValue(value);
        section->AddProperty(property);
    }
}

void EditorUIPackageControlSectionLoader::Apply()
{
    if (section->GetCount() > 0)
    {
        UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
        DVASSERT(component != NULL);
        component->GetPropertiesRoot()->AddProperty(section);
    }
    else
    {
        SafeRelease(section);
    }
}

EditorUIPackageBackgroundSectionLoader::EditorUIPackageBackgroundSectionLoader(UIControl *control, int num) : UIPackageBackgroundSectionLoader(control, num)
{
    section = new BackgroundPropertiesSection(control, num);
    
    UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
    DVASSERT(component != NULL);
    component->GetPropertiesRoot()->AddProperty(section);
}

void EditorUIPackageBackgroundSectionLoader::SetProperty(const InspMember *member, const DAVA::VariantType &value)
{
    /*UIPackageBackgroundSection::SetProperty(member, value);*/
    ValueProperty *property = new ValueProperty(GetBaseObject(), member);
    if (value.GetType() != VariantType::TYPE_NONE)
    {
        property->SetValue(value);
        bgHasChanges = true;
    }
    section->AddProperty(property);
}

void EditorUIPackageBackgroundSectionLoader::Apply()
{
    UIPackageBackgroundSectionLoader::Apply();
    if (bgWasCreated && !bgHasChanges)
        section->HideContent();
}

EditorUIPackageInternalControlSectionLoader::EditorUIPackageInternalControlSectionLoader(UIControl *control, int num) : UIPackageInternalControlSectionLoader(control, num)
{
    section = new InternalControlPropertiesSection(control, num);
    UIEditorComponent *component = dynamic_cast<UIEditorComponent*>(control->GetCustomData());
    DVASSERT(component != NULL);
    component->GetPropertiesRoot()->AddProperty(section);
}

void EditorUIPackageInternalControlSectionLoader::SetProperty(const InspMember *member, const DAVA::VariantType &value)
{
    /*UIPackageInternalControlSection::SetProperty(member, value);*/
    ValueProperty *property = new ValueProperty(GetBaseObject(), member);
    if (value.GetType() != VariantType::TYPE_NONE)
    {
        property->SetValue(value);
        internalHasChanges = true;
    }
    section->AddProperty(property);
}

void EditorUIPackageInternalControlSectionLoader::Apply()
{
    UIPackageInternalControlSectionLoader::Apply();
    if (internalWasCreated && !internalHasChanges)
        section->HideContent();
}
