//
//  UIControlPropertiesGroup.h
//  Framework
//
//  Created by Dmitry Belsky on 19.9.14.
//
//

#ifndef __DAVAENGINE_UI_CONTROL_PROPERTIES_GROUP_H__
#define __DAVAENGINE_UI_CONTROL_PROPERTIES_GROUP_H__

#include "Base/BaseObject.h"

namespace DAVA
{
    class UIControlPropertiesGroup : public BaseObject
    {
    public:
        UIControlPropertiesGroup();
    protected:
        virtual ~UIControlPropertiesGroup();
        
    public:
        INTROSPECTION_EXTEND(UIControlPropertiesGroup, BaseObject,
                             NULL
                        )
    };
}

#endif // __DAVAENGINE_UI_CONTROL_PROPERTIES_GROUP_H__
