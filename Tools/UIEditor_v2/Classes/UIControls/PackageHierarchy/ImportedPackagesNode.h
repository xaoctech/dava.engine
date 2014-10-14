//
//  ImportedPackagesNode.h
//  UIEditor
//
//  Created by Dmitry Belsky on 9.10.14.
//
//

#ifndef __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
#define __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__

#include "PackageBaseNode.h"

class PackageControlsNode;

class ImportedPackagesNode : public PackageBaseNode
{
public:
    ImportedPackagesNode(PackageBaseNode *parent);
    virtual ~ImportedPackagesNode();

    void Add(PackageControlsNode *node);
    virtual int GetCount() const override;
    virtual PackageBaseNode *Get(int index) const override;
    
    virtual DAVA::String GetName() const;
    PackageControlsNode *FindPackageControlsNodeByName(const DAVA::String &name) const;

    
    virtual bool IsHeader() const {
        return true;
    }
    
    virtual bool IsEditable() const {
        return false;
    }
    
private:
    DAVA::Vector<PackageControlsNode*> packageControlsNode;
};



#endif //__UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
