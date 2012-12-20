//
//  BaseMetadataParams.h
//  UIEditor
//
//  Created by Yuri Coder on 10/24/12.
//
//

#ifndef __UIEditor__BaseMetadataParams__
#define __UIEditor__BaseMetadataParams__

#include "HierarchyTreeNode.h"

#include "UI/UIControl.h"

namespace DAVA {
    
// Base Metadata Params.
class BaseMetadataParams
{
public:
    BaseMetadataParams(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID, UIControl* uiControl);
    
    HierarchyTreeNode::HIERARCHYTREENODEID GetTreeNodeID() const;
    UIControl* GetUIControl() const;

    typedef int METADATAPARAMID;
    static const METADATAPARAMID BaseMetadataID_UNKNOWN = -1;

private:
    // Hierarchy Tree node ID.
    HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID;
    
    // UI Control attached.
    UIControl* uiControl;
};

typedef Vector<BaseMetadataParams> METADATAPARAMSVECT;
typedef METADATAPARAMSVECT::iterator METADATAPARAMSITER;
typedef METADATAPARAMSVECT::const_iterator METADATAPARAMSCONSTITER;
};

#endif /* defined(__UIEditor__BaseMetadataParams__) */
