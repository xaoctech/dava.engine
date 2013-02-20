//
//  ScreenControl.h
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//

#ifndef __UIEditor__ScreenControl__
#define __UIEditor__ScreenControl__

#include <DAVAEngine.h>
#include <QString>

using namespace DAVA;

class HierarchyTreeNode;

class ScreenControl: public UIControl
{
public:
	ScreenControl();
	~ScreenControl();
	
	virtual bool IsPointInside(const Vector2& point, bool expandWithFocus);
};

#endif /* defined(__UIEditor__ScreenControl__) */
