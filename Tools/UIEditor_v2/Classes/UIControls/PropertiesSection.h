//
//  PropertiesSection.h
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#ifndef __UI_EDITOR_PROPERTIES_SECTION_H__
#define __UI_EDITOR_PROPERTIES_SECTION_H__

#include "BaseProperty.h"

class PropertiesSection : public BaseProperty
{
public:
    virtual ePropertyType GetType() const {
        return TYPE_HEADER;
    }

};

#endif // __UI_EDITOR_PROPERTIES_SECTION_H__
