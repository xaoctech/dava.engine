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
    PackageBaseNode();
    virtual ~PackageBaseNode();
    
    int GetCount() const;
    PackageBaseNode *Get(int index) const;
    int GetIndex(PackageBaseNode *node) const;
    void Add(PackageBaseNode *node);
    
    PackageBaseNode *GetParent() const;
    
    virtual DAVA::String GetName() const;
    virtual DAVA::UIControl *GetControl() const;
    virtual bool IsHeader() const;
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    virtual bool IsEditable() const;
    
private:
    DAVA::Vector<PackageBaseNode*> children;
    PackageBaseNode *parent;
};




#endif // __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
