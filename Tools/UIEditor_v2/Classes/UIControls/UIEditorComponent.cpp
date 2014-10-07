//
//  UIEditorComponent.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 16.9.14.
//
//

#include "UIEditorComponent.h"

using namespace DAVA;

class PropertiesRoot : public BaseProperty
{
public:
    virtual String GetName() const {
        return "ROOT";
    }
    
    virtual ePropertyType GetType() const {
        return TYPE_HEADER;
    }
    
};

UIEditorComponent::UIEditorComponent() : prototype(NULL), prototypePackage(NULL), clonedFromPrototype(false), propertiesRoot(NULL)
{
    propertiesRoot = new PropertiesRoot();
}

UIEditorComponent::~UIEditorComponent()
{
    SafeRelease(propertiesRoot);
}

UIControl *UIEditorComponent::GetPrototype() const
{
    return prototype;
}

UIPackage *UIEditorComponent::GetPrototypePackage() const
{
    return prototypePackage;
}

void UIEditorComponent::SetPrototype(DAVA::UIControl *prototype, DAVA::UIPackage *package)
{
    SafeRetain(prototype);
    SafeRelease(this->prototype);
    this->prototype = prototype;
    
    SafeRetain(package);
    SafeRelease(prototypePackage);
    prototypePackage = package;

    clonedFromPrototype = true;
}

bool UIEditorComponent::IsClonedFromPrototype() const
{
    return clonedFromPrototype;
}

void UIEditorComponent::SetClonedFromPrototype(const DAVA::String &path)
{
    clonedFromPrototype = true;
    pathFromPrototype = path;
}
