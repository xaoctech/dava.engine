#ifndef __UI_EDITOR_PROPERTIES_ROOT_H__
#define __UI_EDITOR_PROPERTIES_ROOT_H__

#include "UIControls/BaseProperty.h"

class PropertiesSection;

class PropertiesRoot : public BaseProperty
{
public:
    PropertiesRoot();
    virtual ~PropertiesRoot();
    
    void AddProperty(PropertiesSection *section);
    virtual int GetCount() const override;
    virtual BaseProperty *GetProperty(int index) const override;
    
    PropertiesRoot *CopyAndApplyForNewControl(DAVA::UIControl *control);
    
    virtual DAVA::String GetName() const;
    virtual ePropertyType GetType() const;
    
private:
    DAVA::Vector<PropertiesSection*> children;
};

#endif // __UI_EDITOR_PROPERTIES_ROOT_H__
