//
//  HierarchyTreeAggregatorControlNode.h
//  UIEditor
//
//  Created by adebt on 3/13/13.
//
//

#ifndef __UIEditor__HierarchyTreeAggregatorControlNode__
#define __UIEditor__HierarchyTreeAggregatorControlNode__

#include "HierarchyTreeControlNode.h"

namespace DAVA
{
	class HierarchyTreeAggregatorNode;
	
	class HierarchyTreeAggregatorControlNode: public HierarchyTreeControlNode
	{
	public:
		HierarchyTreeAggregatorControlNode(HierarchyTreeAggregatorNode* parentAggregator,
										   HierarchyTreeNode* parent,
										   UIControl* uiObject,
										   const QString& name);
		HierarchyTreeAggregatorControlNode(HierarchyTreeNode* parent, const HierarchyTreeAggregatorControlNode* node);
		
		~HierarchyTreeAggregatorControlNode();
		
		void SetAggregatorNode(HierarchyTreeAggregatorNode* parentAggregator);
		const HierarchyTreeAggregatorNode* GetAggregatorNode() const {return parentAggregator;}
		virtual void RemoveTreeNodeFromScene();
		virtual void ReturnTreeNodeToScene();

		String GetAggregatorPath() const;
	private:
		HierarchyTreeAggregatorNode* parentAggregator;
		HierarchyTreeAggregatorNode* parentAggregatorSave;
	};
}

#endif /* defined(__UIEditor__HierarchyTreeAggregatorControlNode__) */
