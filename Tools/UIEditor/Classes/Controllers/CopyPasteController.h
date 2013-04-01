//
//  CopyPasteController.h
//  UIEditor
//
//  Created by adebt on 11/5/12.
//
//

#ifndef __UIEditor__CopyPasteController__
#define __UIEditor__CopyPasteController__

#include "DAVAEngine.h"


#include "HierarchyTreeController.h"

namespace DAVA {
	class CopyPasteController: public Singleton<CopyPasteController>
	{
	public:
		enum CopyType
		{
			CopyTypeNone,
			CopyTypeControl,
			CopyTypeScreen,
			CopyTypePlatform,
		};
		
	public:
		CopyPasteController();
		~CopyPasteController();
		
		CopyType GetCopyType() const;
		
		void CopyControls(const HierarchyTreeController::SELECTEDCONTROLNODES& items);
		void Copy(const HierarchyTreeNode::HIERARCHYTREENODESLIST& items);
		bool ControlIsChild(const HierarchyTreeController::SELECTEDCONTROLNODES& items, const HierarchyTreeControlNode* control) const;
		
		void Paste(HierarchyTreeNode* parentNode);
		
	private:
		void Clear();
				
	private:
		HierarchyTreeNode::HIERARCHYTREENODESLIST items;
		CopyType copyType;
	};
}

#endif /* defined(__UIEditor__CopyPasteController__) */
