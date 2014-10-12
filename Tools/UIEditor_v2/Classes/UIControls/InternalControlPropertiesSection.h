#ifndef __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class InternalControlPropertiesSection : public PropertiesSection
{
public:
    InternalControlPropertiesSection(DAVA::UIControl *control, int num);
    virtual ~InternalControlPropertiesSection();
    
    virtual PropertiesSection *CopyAndApplyForNewControl(DAVA::UIControl *newControl) override;

    virtual DAVA::String GetName() const;

    virtual int GetCount() const;
    void HideContent();

private:
    DAVA::UIControl *control;
    int internalControlNum;
    bool isContentHidden;
};


#endif // __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
