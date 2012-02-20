/*
 *  TestScreen.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIFileTreeCell.h"

#include "SceneEditor/ControlsFactory.h"

namespace DAVA 
{
UIFileTreeCell::UIFileTreeCell(const Rect &rect, const String &cellIdentifier)
:	UIListCell(rect, cellIdentifier)
    ,   text(NULL)
    ,   openButton(NULL)
{
    text = new UIStaticText(Rect(15, 0, rect.dx - 15, rect.dy));
    AddControl(text);
    openButton = new UIButton(Rect(0, 0, 15, rect.dy));
    openButton->SetInputEnabled(false);
    AddControl(openButton);
}
	
UIFileTreeCell::~UIFileTreeCell()
{
    SafeRelease(text);
    SafeRelease(openButton);
}
    
void UIFileTreeCell::SetItemInfo(UITreeItemInfo * _itemInfo)
{
	itemInfo = _itemInfo;
}

UITreeItemInfo * UIFileTreeCell::GetItemInfo()
{
	return itemInfo;
}
	
};

