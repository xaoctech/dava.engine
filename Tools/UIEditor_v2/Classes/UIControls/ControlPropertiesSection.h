//
//  ControlPropertiesSection.h
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#ifndef __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class ControlPropertiesSection : public PropertiesSection
{
public:
    ControlPropertiesSection(const DAVA::String &name);
    
    virtual DAVA::String GetName() const;

private:
    DAVA::String name;
};

#endif // __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
