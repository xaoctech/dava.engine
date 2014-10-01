//
//  InternalControlPropertiesSection.h
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#ifndef __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class InternalControlPropertiesSection : public PropertiesSection
{
public:
    InternalControlPropertiesSection(DAVA::UIControl *control, int num);
    virtual ~InternalControlPropertiesSection();
    
    virtual DAVA::String GetName() const;
    
private:
    DAVA::UIControl *control;
    int internalControlNum;
};


#endif // __UI_EDITOR_INTERNAL_CONTROL_PROPERTIES_SECTION_H__
