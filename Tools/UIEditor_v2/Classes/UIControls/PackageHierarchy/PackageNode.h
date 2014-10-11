#ifndef __UI_EDITOR_PACKAGE_NODE_H__
#define __UI_EDITOR_PACKAGE_NODE_H__

#include "PackageBaseNode.h"

class PackageNode : public PackageBaseNode
{
public:
    PackageNode(DAVA::UIPackage *package);
    virtual ~PackageNode();
    
    virtual DAVA::String GetName() const;
    
    virtual bool IsHeader() const {return true; }
    
private:
    DAVA::UIPackage *package;
};

#endif // __UI_EDITOR_PACKAGE_NODE_H__
