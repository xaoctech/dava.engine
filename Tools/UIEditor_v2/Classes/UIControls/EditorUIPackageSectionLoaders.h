//
//  EditorUIPackageSectionLoaders.h
//  UIEditor
//
//  Created by Dmitry Belsky on 8.10.14.
//
//

#ifndef __UI_EDITOR_UI_PACKAGE_SECTION_LOADERS_H__
#define __UI_EDITOR_UI_PACKAGE_SECTION_LOADERS_H__

#include "UI/UIPackageSectionLoader.h"

class PropertiesSection;
class BackgroundPropertiesSection;
class InternalControlPropertiesSection;

class EditorUIPackageControlSectionLoader : public DAVA::UIPackageControlSectionLoader
{
public:
    EditorUIPackageControlSectionLoader(DAVA::UIControl *control, const DAVA::String &name);
    virtual void SetProperty(const DAVA::InspMember *member, const DAVA::VariantType &value) override;
    
    virtual void Apply() override;
    
private:
    PropertiesSection *section;
};

class EditorUIPackageBackgroundSectionLoader : public DAVA::UIPackageBackgroundSectionLoader
{
public:
    EditorUIPackageBackgroundSectionLoader(DAVA::UIControl *control, int num);
    virtual void SetProperty(const DAVA::InspMember *member, const DAVA::VariantType &value) override;
    virtual void Apply() override;
    
private:
    BackgroundPropertiesSection *section;
};

class EditorUIPackageInternalControlSectionLoader : public DAVA::UIPackageInternalControlSectionLoader
{
public:
    EditorUIPackageInternalControlSectionLoader(DAVA::UIControl *control, int num);
    virtual void SetProperty(const DAVA::InspMember *member, const DAVA::VariantType &value) override;
    virtual void Apply() override;
    
private:
    InternalControlPropertiesSection *section;
};


#endif // __UI_EDITOR_UI_PACKAGE_SECTION_LOADERS_H__
