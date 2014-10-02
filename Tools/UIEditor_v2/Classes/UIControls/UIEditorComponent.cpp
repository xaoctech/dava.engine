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

UIEditorComponent::UIEditorComponent() : prototype(NULL), clonedFromPrototype(false), propertiesRoot(NULL)
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

void UIEditorComponent::SetPrototype(DAVA::UIControl *prototype)
{
    SafeRetain(prototype);
    SafeRelease(this->prototype);
    this->prototype = prototype;

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
