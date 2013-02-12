//
//  ScreenControl.cpp
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//

#include "ScreenControl.h"
#include <QString>
#include "HierarchyTreeNode.h"

ScreenControl::ScreenControl()
{
	UIControlBackground* bkg = new UIControlBackground();
	bkg->SetColor(Color(0.2f, 0.2f, 0.2f, 1.0f));
	bkg->SetDrawType(UIControlBackground::DRAW_FILL);
	SetBackground(bkg);
}

ScreenControl::~ScreenControl()
{
	
}

bool ScreenControl::IsPointInside(const Vector2& /*point*/, bool/* expandWithFocus*/)
{
	//YZ:
	//input must be handled by screen
	return false;
}