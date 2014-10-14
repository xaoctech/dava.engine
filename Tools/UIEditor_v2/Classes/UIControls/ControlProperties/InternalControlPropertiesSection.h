#ifndef __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class InternalControlPropertiesSection : public PropertiesSection
{
public:
    InternalControlPropertiesSection(DAVA::UIControl *control, int num, const InternalControlPropertiesSection *sourceSection);
    virtual ~InternalControlPropertiesSection();

    virtual DAVA::UIControl *GetInternalControl() const;
    void CreateInternalControl();
    
    virtual DAVA::String GetName() const;

    virtual int GetCount() const;
    void HideContent();

private:
    DAVA::UIControl *control;
    DAVA::UIControl *internalControl;
    int internalControlNum;
    bool isContentHidden;
};


#endif // __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
