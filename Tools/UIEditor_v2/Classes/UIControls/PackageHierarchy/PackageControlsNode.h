#ifndef __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
#define __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__

#include "PackageBaseNode.h"
#include "ControlNode.h"

class PackageControlsNode : public PackageBaseNode
{
public:
    PackageControlsNode(PackageBaseNode *parent);
    virtual ~PackageControlsNode();
    
    void Add(ControlNode *node);
    virtual int GetCount() const override;
    virtual ControlNode *Get(int index) const override;

    virtual DAVA::String GetName() const;
    void SetName(const DAVA::String &name);
    
    virtual bool IsHeader() const {
        return true;
    }
    
    virtual bool IsEditable() const {
        return editable;
    }
    
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;

    ControlNode *FindControlNodeByName(const DAVA::String &name) const;
    
private:
    DAVA::String name;
    bool editable;
    DAVA::Vector<ControlNode*> nodes;
};

#endif // __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
