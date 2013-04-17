//
//  MetadataFactory.h
//  UIEditor
//
//  Created by Yuri Coder on 10/18/12.
//
//

#ifndef __UIEditor__MetadataFactory__
#define __UIEditor__MetadataFactory__

#include "MetadataFactory.h"
#include "BaseMetadata.h"

#include "PlatformMetadata.h"
#include "ScreenMetadata.h"
#include "AggregatorMetadata.h"
#include "HierarchyTreeController.h"

#include "Base/Singleton.h"
#include "UI/UIControl.h"

namespace DAVA {
    
// Metadata Factory is responsible for returning metadata for particular DAVA UI class.
class MetadataFactory : public Singleton<MetadataFactory>
{
public:
    MetadataFactory();
    virtual ~MetadataFactory();
    
    // Get the metadata for the UIControl based on its type.
    BaseMetadata* GetMetadataForUIControl(const UIControl* uiControl) const;

    // Get the metadata for the tree node based on its type or ID.
    BaseMetadata* GetMetadataForTreeNode(const HierarchyTreeNode* treeNode) const;
    BaseMetadata* GetMetadataForTreeNode(HierarchyTreeNode::HIERARCHYTREENODEID treeNodeID) const;

    // Get the common metadata for list of Hierarchy Tree Control nodes.
    BaseMetadata* GetMetadataForTreeNodesList(const HierarchyTreeController::SELECTEDCONTROLNODES& nodesList) const;

protected:
    // Get the specific metadata for non-UI Controls.
    PlatformMetadata* GetPlatformMetadata() const;
    ScreenMetadata* GetScreenMetadata() const;
	AggregatorMetadata* GetAggregatorMetadata() const;
};
    
}

#endif /* defined(__UIEditor__MetadataFactory__) */
