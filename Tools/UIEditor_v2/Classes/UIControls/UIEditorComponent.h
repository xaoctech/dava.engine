//
//  UIEditorComponent.h
//  UIEditor
//
//  Created by Dmitry Belsky on 16.9.14.
//
//

#ifndef __UIEDITOR_UI_EDITOR_COMPONENT__
#define __UIEDITOR_UI_EDITOR_COMPONENT__

#include "UI/UIControl.h"
#include "BaseProperty.h"

class UIEditorComponent : public DAVA::BaseObject
{
public:
    UIEditorComponent();
private:
    virtual ~UIEditorComponent();
public:
    DAVA::UIControl *GetPrototype() const;
    void SetPrototype(DAVA::UIControl *prototype);
    
    bool IsClonedFromPrototype() const;
    void SetClonedFromPrototype(const DAVA::String &path);
    const DAVA::String &GetPathFromPrototype() const {return pathFromPrototype; }

    BaseProperty *GetPropertiesRoot() const {return propertiesRoot; }
    
private:
    DAVA::UIControl *prototype;
    DAVA::String pathFromPrototype;
    bool clonedFromPrototype;
    
    BaseProperty *propertiesRoot;
};

#endif // __UIEDITOR_UI_EDITOR_COMPONENT__
