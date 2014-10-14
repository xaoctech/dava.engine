//
//  UIPackageModelNode.h
//  UIEditor
//
//  Created by Dmitry Belsky on 3.10.14.
//
//

#ifndef __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
#define __UI_EDITOR_UI_PACKAGE_MODEL_NODE__

#include "DAVAEngine.h"

class PackageBaseNode : public DAVA::BaseObject
{
public:
    PackageBaseNode(PackageBaseNode *parent);
    virtual ~PackageBaseNode();
    
    virtual int GetCount() const = 0;
    virtual PackageBaseNode *Get(int index) const = 0;
    int GetIndex(PackageBaseNode *node) const;
    
    PackageBaseNode *GetParent() const;
    void SetParent(PackageBaseNode *parent);
    
    virtual DAVA::String GetName() const;
    virtual DAVA::UIControl *GetControl() const;
    virtual bool IsHeader() const;
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    virtual bool IsEditable() const;
    
    virtual void debugDump(int depth);
    
private:
    PackageBaseNode *parent;
};




#endif // __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
