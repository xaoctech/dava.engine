#ifndef __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__

#include "AbstractUIPackageBuilder.h"
#include "UIPackage.h"

namespace DAVA
{
    class DefaultUIPackageBuilder : public AbstractUIPackageBuilder
    {
    public:
        DefaultUIPackageBuilder();
        virtual ~DefaultUIPackageBuilder();
        
        virtual UIPackage *BeginPackage(const FilePath &packagePath) override;
        virtual void EndPackage() override;
        
        virtual UIPackage *ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader) override;
        
        virtual UIControl *BeginControlWithClass(const String className) override;
        virtual UIControl *BeginControlWithCustomClass(const String customClassName, const String className) override;
        virtual UIControl *BeginControlWithPrototype(const String &packageName, const String &prototypeName, AbstractUIPackageLoader *loader) override;
        virtual UIControl *BeginControlWithPath(const String &pathName) override;
        virtual UIControl *BeginUnknownControl(const YamlNode *node) override;
        virtual void EndControl() override;
        
        virtual void BeginControlPropretiesSection(const String &name) override;
        virtual void EndControlPropertiesSection() override;
        
        virtual UIControlBackground *BeginBgPropertiesSection(int index, bool sectionHasProperties) override;
        virtual void EndBgPropertiesSection() override;
        
        virtual UIControl *BeginInternalControlSection(int index, bool sectionHasProperties) override;
        virtual void EndInternalControlSection() override;
        
        virtual void ProcessProperty(const InspMember *member, const VariantType &value) override;
        
    private:
        void AddControl(UIControl *control);
        
    private:
        UIPackage *package;
        Map<String, UIPackage*> importedPackages;
        Vector<UIControl*> controlsStack;
        BaseObject *currentObject;
    };
}

#endif // __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__
