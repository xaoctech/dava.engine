#ifndef __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
#define __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__

#include "PackageBaseNode.h"

class PackageControlsNode : public PackageBaseNode
{
public:
    PackageControlsNode(DAVA::UIPackage *package, const DAVA::String &name, bool editable);
    virtual ~PackageControlsNode();
    
    virtual DAVA::String GetName() const;
    
    virtual bool IsHeader() const {
        return true;
    }
    
    virtual bool IsEditable() const {
        return editable;
    }
    
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    
private:
    DAVA::UIPackage *package;
    const DAVA::String name;
    bool editable;
};

#endif // __UI_EDITOR_PACKAGE_CONTROLS_NODE_H__
