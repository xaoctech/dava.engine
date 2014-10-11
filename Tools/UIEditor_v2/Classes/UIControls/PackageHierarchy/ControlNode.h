//
//  ControlNode.h
//  UIEditor
//
//  Created by Dmitry Belsky on 9.10.14.
//
//

#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"

#include "UIControls/BaseProperty.h"

class ControlNode : public PackageBaseNode
{
public:
    ControlNode(DAVA::UIControl *control, bool editable);
    virtual ~ControlNode();
    
    virtual DAVA::String GetName() const;
    DAVA::UIControl *GetControl() const;
    
    virtual bool IsHeader() const {return false; }
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    virtual bool IsEditable() const {return editable; }

    BaseProperty *GetPropertiesRoot() const {return propertiesRoot; }

private:
    DAVA::UIControl *control;
    BaseProperty *propertiesRoot;
    
    bool editable;
};


#endif // __UI_EDITOR_CONTROL_NODE__
