#include "DefaultUIPackageBuilder.h"

#include "UIPackage.h"
#include "UIPackageLoader.h"
#include "Base/ObjectFactory.h"
#include "UI/UIControl.h"
#include "UI/UIControlHelpers.h"

namespace DAVA
{
    DefaultUIPackageBuilder::DefaultUIPackageBuilder()
    : package(NULL), currentObject(NULL)
    {
        
    }
    
    DefaultUIPackageBuilder::~DefaultUIPackageBuilder()
    {
        for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
            it->second->Release();
        importedPackages.clear();
    }
 
    UIPackage *DefaultUIPackageBuilder::BeginPackage(const FilePath &packagePath)
    {
        DVASSERT(package == NULL)
        package = new UIPackage(packagePath);
        return package;
    }
    
    void DefaultUIPackageBuilder::EndPackage()
    {
        package = NULL;
    }
    
    UIPackage *DefaultUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
    {
        UIPackage *result;
        UIPackage *prevPackage = package;
        package = NULL;
        auto it = importedPackages.find(packagePath);
        if (it != importedPackages.end())
        {
            result = it->second;
            prevPackage->AddPackage(it->second);
        }
        else
        {
            UIPackage *loadedPackage = loader->LoadPackage(packagePath);
            result = loadedPackage;
            importedPackages[packagePath] = loadedPackage;
            prevPackage->AddPackage(loadedPackage);
        }
        DVASSERT(package == NULL);
        package = prevPackage;
        return result;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginControlWithClass(const String className)
    {
        UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
        if (!control)
            Logger::Warning("[DefaultUIControlFactory::CreateControl] Can't create control with class name \"%s\"", className.c_str());

        AddControl(control);
        return control;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginControlWithCustomClass(const String customClassName, const String className)
    {
        UIControl *control = ObjectFactory::Instance()->New<UIControl>(customClassName);

        if (!control)
            control = ObjectFactory::Instance()->New<UIControl>(className); // TODO: remove
        
        if (control)
        {
            control->SetCustomControlClassName(customClassName);
        }
        else
        {
            DVASSERT(control != NULL);
        }
        
        AddControl(control);
        return control;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, AbstractUIPackageLoader *loader)
    {
        UIControl *prototype;
        if (packageName.empty())
        {
            prototype = package->GetControl(prototypeName);
            if (!prototype)
            {
                if (loader->LoadControlByName(prototypeName))
                    prototype = package->GetControl(prototypeName);
            }
        }
        else
        {
            UIPackage *importedPackage = package->GetPackage(packageName);
            if (importedPackage)
                prototype = importedPackage->GetControl(prototypeName);
        }
        
        DVASSERT(prototype != NULL);
        
        UIControl *control = prototype->Clone();
        AddControl(control);
        return control;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginControlWithPath(const String &pathName)
    {
        UIControl *control = NULL;
        if (!controlsStack.empty())
        {
            control = UIControlHelpers::GetControlByPath(pathName, controlsStack.back());
        }
        
        DVASSERT(control != NULL);
        controlsStack.push_back(SafeRetain(control));
        return control;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
    {
        DVASSERT(false);
        controlsStack.push_back(NULL);
        return NULL;
    }
    
    void DefaultUIPackageBuilder::EndControl()
    {
        UIControl *lastControl = controlsStack.back();
        controlsStack.pop_back();
        if (lastControl)
            SafeRelease(lastControl);
    }
    
    void DefaultUIPackageBuilder::BeginControlPropretiesSection(const String &name)
    {
        currentObject = controlsStack.back();
    }
    
    void DefaultUIPackageBuilder::EndControlPropertiesSection()
    {
        currentObject = NULL;
    }
    
    UIControlBackground *DefaultUIPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
    {
        if (sectionHasProperties)
        {
            UIControl *control = controlsStack.back();
            if (!control->GetBackgroundComponent(index))
            {
                UIControlBackground *bg = control->CreateBackgroundComponent(index);
                control->SetBackgroundComponent(index, bg);
                SafeRelease(bg);
            }
            UIControlBackground *res = control->GetBackgroundComponent(index);
            currentObject = res;
            return res;
        }
        return NULL;
    }
    
    void DefaultUIPackageBuilder::EndBgPropertiesSection()
    {
        currentObject = NULL;
    }
    
    UIControl *DefaultUIPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
    {
        if (sectionHasProperties)
        {
            UIControl *control = controlsStack.back();
            if (!control->GetInternalControl(index))
            {
                UIControl *internal = control->CreateInternalControl(index);
                control->SetInternalControl(index, internal);
                SafeRelease(internal);
            }
            UIControl *res = control->GetInternalControl(index);
            currentObject = res;
            return res;
        }
        return NULL;
    }
    
    void DefaultUIPackageBuilder::EndInternalControlSection()
    {
        currentObject = NULL;
    }
    
    void DefaultUIPackageBuilder::ProcessProperty(const InspMember *member, const VariantType &value)
    {
        DVASSERT(currentObject);
        
        if (currentObject && value.GetType() != VariantType::TYPE_NONE)
            member->SetValue(currentObject, value);
    }

    void DefaultUIPackageBuilder::AddControl(UIControl *control)
    {
        if (control)
        {
            if (controlsStack.empty())
                package->AddControl(control);
            else
                controlsStack.back()->AddControl(control);
        }
        
        controlsStack.push_back(control);
    }

}
