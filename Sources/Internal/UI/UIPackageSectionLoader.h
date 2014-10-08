#ifndef __DAVAENGINE_UI_PACKAGE_SECTION_LOADER_H__
#define __DAVAENGINE_UI_PACKAGE_SECTION_LOADER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA
{
    class UIYamlLoader;
    class UIControl;
    class YamlNode;
    class FilePath;
    class UIPackage;
    class UIControlFactory;
    class UIControlBackground;

    ////////////////////////////////////////////////////////////////////////////////
    // UIPackageSectionIControlSection
    ////////////////////////////////////////////////////////////////////////////////
    
    class UIPackageSectionLoader : public BaseObject
    {
    public:
        UIPackageSectionLoader();
        virtual ~UIPackageSectionLoader();
        
        virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value) = 0;
        virtual BaseObject *GetBaseObject() const = 0;
        virtual String GetName() const = 0;
        
        virtual void Apply() = 0;
    };
    
    class UIPackageControlSectionLoader : public UIPackageSectionLoader
    {
    public:
        UIPackageControlSectionLoader(UIControl *control, const String &name);
        virtual ~UIPackageControlSectionLoader();
        
        virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value);
        virtual BaseObject *GetBaseObject() const;
        virtual String GetName() const;
        
        virtual void Apply();
        
    protected:
        String name;
        UIControl *control;
    };
    
    class UIPackageBackgroundSectionLoader : public UIPackageSectionLoader
    {
    public:
        UIPackageBackgroundSectionLoader(UIControl *control, int num);
        virtual ~UIPackageBackgroundSectionLoader();
        
        virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value);
        virtual BaseObject *GetBaseObject() const;
        virtual String GetName() const;
        
        virtual void Apply();
        
    protected:
        UIControl *control;
        UIControlBackground *bg;
        bool bgWasCreated;
        bool bgHasChanges;
        int bgNum;
    };
    
    class UIPackageInternalControlSectionLoader : public UIPackageSectionLoader
    {
    public:
        UIPackageInternalControlSectionLoader(UIControl *control, int num);
        virtual ~UIPackageInternalControlSectionLoader();
        
        virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value);
        virtual BaseObject *GetBaseObject() const;
        virtual String GetName() const;
        
        virtual void Apply();
        
    protected:
        UIControl *control;
        UIControl *internalControl;
        bool internalWasCreated;
        bool internalHasChanges;
        int internalControlNum;
    };

}

#endif //__DAVAENGINE_UI_PACKAGE_SECTION_LOADER_H__
