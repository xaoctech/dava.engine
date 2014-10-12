#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"

#include "UIControls/ControlProperties/PropertiesRoot.h"

class ControlNode : public PackageBaseNode
{
public:
    ControlNode(DAVA::UIControl *control);
    ControlNode(const ControlNode *node);
    virtual ~ControlNode();

    void Add(ControlNode *node);
    virtual int GetCount() const override;
    virtual PackageBaseNode *Get(int index) const override;
    ControlNode *FindByName(const DAVA::String &name) const;
    
    ControlNode *Clone() const;
    
    virtual DAVA::String GetName() const;
    DAVA::UIControl *GetControl() const;
    
    virtual bool IsHeader() const {return false; }
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    virtual bool IsEditable() const {return editable; }

    PropertiesRoot *GetPropertiesRoot() const {return propertiesRoot; }

private:
    DAVA::UIControl *control;
    PropertiesRoot *propertiesRoot;
    DAVA::Vector<ControlNode*>nodes;
    
    bool editable;
    bool cloned;
    
};


#endif // __UI_EDITOR_CONTROL_NODE__
