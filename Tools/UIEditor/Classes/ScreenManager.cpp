//
//  ScreenManager.cpp
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//

#include "ScreenManager.h"
#include "DefaultScreen.h"
#include "HierarchyTreeController.h"

ScreenManager::ScreenManager()
{
	defaultScreen = new DefaultScreen();
	UIScreenManager::Instance()->RegisterScreen(DEFAULT_SCREEN, defaultScreen);
	UIScreenManager::Instance()->SetFirst(DEFAULT_SCREEN);

	activeScreenControl = NULL;
	
	connect(HierarchyTreeController::Instance(),
			SIGNAL(SelectedScreenChanged(const HierarchyTreeScreenNode*)),
			this,
			SLOT(OnSelectedScreenChanged(const HierarchyTreeScreenNode*)));
}

ScreenManager::~ScreenManager()
{
	SafeRelease(defaultScreen);
}

void ScreenManager::OnSelectedScreenChanged(const HierarchyTreeScreenNode* node)
{
	if (activeScreenControl)
	{
		defaultScreen->RemoveControl(activeScreenControl);
		activeScreenControl = NULL;	
	}
	
	if (node)
	{
		ScreenControl* screen = node->GetScreen();
		if (screen)
		{
			activeScreenControl = screen;
			defaultScreen->AddControl(activeScreenControl);
		}
	}
}