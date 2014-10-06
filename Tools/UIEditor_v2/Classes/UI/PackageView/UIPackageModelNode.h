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

class UIPackageModelNode : public DAVA::BaseObject
{
public:
    UIPackageModelNode();
    virtual ~UIPackageModelNode();
    
    int GetCount() const;
    UIPackageModelNode *Get(int index) const;
    int GetIndex(UIPackageModelNode *node) const;
    void Add(UIPackageModelNode *node);
    
    UIPackageModelNode *GetParent() const;
    
    virtual DAVA::String GetName() const;
    virtual DAVA::UIControl *GetControl() const;
    virtual bool IsHeader() const;
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    virtual bool IsEditable() const;
    
private:
    DAVA::Vector<UIPackageModelNode*> children;
    UIPackageModelNode *parent;
};

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelRootNode
////////////////////////////////////////////////////////////////////////////////

class UIPackageModelRootNode : public UIPackageModelNode
{
public:
    enum eMode
    {
        MODE_ONLY_CONTROLS,
        MODE_CONTROLS_AND_IMPORTED_PACKAGED
    };
public:
    UIPackageModelRootNode(DAVA::UIPackage *package, const DAVA::String &name, eMode mode);
    virtual ~UIPackageModelRootNode();
    
    virtual DAVA::String GetName() const;
    
    virtual bool IsHeader() const {return true; }
    
private:
    DAVA::UIPackage *package;
    DAVA::String name;
};


////////////////////////////////////////////////////////////////////////////////
// UIPackageModelControlNode
////////////////////////////////////////////////////////////////////////////////

class UIPackageModelControlNode : public UIPackageModelNode
{
public:
    UIPackageModelControlNode(DAVA::UIControl *control, bool editable);
    virtual ~UIPackageModelControlNode();

    virtual DAVA::String GetName() const;
    DAVA::UIControl *GetControl() const;

    virtual bool IsHeader() const {return false; }
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;
    virtual bool IsEditable() const {return editable; }

private:
    DAVA::UIControl *control;
    bool editable;
};

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelControlsHeaderNode
////////////////////////////////////////////////////////////////////////////////

class UIPackageModelControlsHeaderNode : public UIPackageModelNode
{
public:
    UIPackageModelControlsHeaderNode(DAVA::UIPackage *package);
    virtual ~UIPackageModelControlsHeaderNode();

    virtual DAVA::String GetName() const;

    virtual bool IsHeader() const {
        return true;
    }
    
    virtual bool IsEditable() const {
        return true;
    }
    
    virtual bool IsInstancedFromPrototype() const;
    virtual bool IsCloned() const;

private:
    DAVA::UIPackage *package;
};

////////////////////////////////////////////////////////////////////////////////
// UIPackageModelImportedPackagesNode
////////////////////////////////////////////////////////////////////////////////

class UIPackageModelImportedPackagesNode : public UIPackageModelNode
{
public:
    UIPackageModelImportedPackagesNode(DAVA::UIPackage *package);
    virtual ~UIPackageModelImportedPackagesNode();
    
    virtual DAVA::String GetName() const;
    
    virtual bool IsHeader() const {
        return true;
    }
    
    virtual bool IsEditable() const {
        return true;
    }
};


#endif // __UI_EDITOR_UI_PACKAGE_MODEL_NODE__
