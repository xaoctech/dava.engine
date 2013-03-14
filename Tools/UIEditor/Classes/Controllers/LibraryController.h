//
//  LibraryController.h
//  UIEditor
//
//  Created by adebt on 3/11/13.
//
//

#ifndef __UIEditor__LibraryController__
#define __UIEditor__LibraryController__

#include <DAVAEngine.h>
#include "HierarchyTreeAggregatorNode.h"
#include "librarywidget.h"

namespace DAVA
{
	class LibraryController: public Singleton<LibraryController>
	{
	public:
		LibraryController();
		~LibraryController();
		
		void Init(LibraryWidget* widget);
		bool IsNameAvailable(const QString& name) const;
		
		void AddControl(HierarchyTreeAggregatorNode* node);
		void RemoveControl(HierarchyTreeAggregatorNode* node);
		void UpdateControl(HierarchyTreeAggregatorNode* node);
		
		HierarchyTreeControlNode* CreateNewControl(HierarchyTreeNode* parentNode, const QString& strType, const QString& name, const Vector2& position);
		
	private:
		void AddControl(const QString& name, DAVA::UIControl* control);
		
	private:
		typedef Map<QString, HierarchyTreeNode*> CONTROLS;
		CONTROLS controls;
		LibraryWidget* widget;
		
		UIAggregatorControl* aggregatorTemp;
	};
}

#endif /* defined(__UIEditor__LibraryController__) */
