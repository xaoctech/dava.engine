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

class ImportedPackagesNode : public PackageBaseNode
{
public:
    ImportedPackagesNode(DAVA::UIPackage *package);
    virtual ~ImportedPackagesNode();
    
    virtual DAVA::String GetName() const;
    
    virtual bool IsHeader() const {
        return true;
    }
    
    virtual bool IsEditable() const {
        return false;
    }
};



#endif //__UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
