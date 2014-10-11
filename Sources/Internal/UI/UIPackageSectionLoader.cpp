#include "UIPackageSectionLoader.h"

#include "UI/UIControl.h"

namespace DAVA
{
    ////////////////////////////////////////////////////////////////////////////////
    // UIPackageSection
    ////////////////////////////////////////////////////////////////////////////////
    
    UIPackageSectionLoader::UIPackageSectionLoader()
    {
    }
    
    UIPackageSectionLoader::~UIPackageSectionLoader()
    {
        
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // UIPackageControlSectionLoader
    ////////////////////////////////////////////////////////////////////////////////
    
    UIPackageControlSectionLoader::UIPackageControlSectionLoader(UIControl *control, const String &name) : control(NULL), name(name)
    {
        this->control = SafeRetain(control);
    }
    
    UIPackageControlSectionLoader::~UIPackageControlSectionLoader()
    {
        SafeRelease(control);
    }
    
    void UIPackageControlSectionLoader::SetProperty(const InspMember *member, const DAVA::VariantType &value)
    {
        if (value.GetType() != VariantType::TYPE_NONE)
            member->SetValue(control, value);
    }
    
    BaseObject *UIPackageControlSectionLoader::GetBaseObject() const
    {
        return control;
    }
    
    String UIPackageControlSectionLoader::GetName() const
    {
        return name;
    }
    
    void UIPackageControlSectionLoader::Apply()
    {
        // do nothing
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // UIPackageBackgroundSectionLoader
    ////////////////////////////////////////////////////////////////////////////////
    
    UIPackageBackgroundSectionLoader::UIPackageBackgroundSectionLoader(UIControl *control, int num) : control(NULL), bg(NULL), bgWasCreated(false), bgHasChanges(false), bgNum(num)
    {
        this->control = SafeRetain(control);
        bg = SafeRetain(control->GetBackgroundComponent(num));
        if (!bg)
        {
            bg = control->CreateBackgroundComponent(num);
            bgWasCreated = true;
        }
    }
    
    UIPackageBackgroundSectionLoader::~UIPackageBackgroundSectionLoader()
    {
        SafeRelease(control);
        SafeRelease(bg);
    }
    
    void UIPackageBackgroundSectionLoader::SetProperty(const InspMember *member, const DAVA::VariantType &value)
    {
        if (value.GetType() != VariantType::TYPE_NONE)
        {
            member->SetValue(bg, value);
            bgHasChanges = true;
        }
    }
    
    BaseObject *UIPackageBackgroundSectionLoader::GetBaseObject() const
    {
        return bg;
    }
    
    String UIPackageBackgroundSectionLoader::GetName() const
    {
        return control->GetBackgroundComponentName(bgNum);
    }
    
    void UIPackageBackgroundSectionLoader::Apply()
    {
        if (bgWasCreated && bgHasChanges)
            control->SetBackgroundComponent(bgNum, bg);
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // UIPackageInternalControlSectionLoader
    ////////////////////////////////////////////////////////////////////////////////
    
    UIPackageInternalControlSectionLoader::UIPackageInternalControlSectionLoader(UIControl *control, int num)
    : control(NULL)
    , internalControl(NULL)
    , internalWasCreated(false)
    , internalHasChanges(false)
    , internalControlNum(num)
    {
        this->control = SafeRetain(control);
        internalControl = SafeRetain(control->GetInternalControl(num));
        if (!internalControl)
        {
            internalControl = control->CreateInternalControl(num);
            internalWasCreated = true;
        }
    }
    
    UIPackageInternalControlSectionLoader::~UIPackageInternalControlSectionLoader()
    {
        SafeRelease(control);
        SafeRelease(internalControl);
    }
    
    void UIPackageInternalControlSectionLoader::SetProperty(const InspMember *member, const DAVA::VariantType &value)
    {
        if (value.GetType() != VariantType::TYPE_NONE)
        {
            member->SetValue(internalControl, value);
            internalHasChanges = true;
        }
    }
    
    BaseObject *UIPackageInternalControlSectionLoader::GetBaseObject() const
    {
        return internalControl;
    }
    
    String UIPackageInternalControlSectionLoader::GetName() const
    {
        return control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();
    }
    
    void UIPackageInternalControlSectionLoader::Apply()
    {
        if (internalWasCreated && internalHasChanges)
            control->SetInternalControl(internalControlNum, internalControl);
    }
}
