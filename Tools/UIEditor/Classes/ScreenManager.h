//
//  ScreenManager.h
//  UIEditor
//
//  Created by adebt on 10/24/12.
//
//

#ifndef __UIEditor__ScreenManager__
#define __UIEditor__ScreenManager__

#include <DAVAEngine.h>
#include "HierarchyTreeScreenNode.h"

#include <QObject>


using namespace DAVA;

#define DEFAULT_SCREEN 0

class ScreenManager: public QObject, public Singleton<ScreenManager>
{
	Q_OBJECT
	
public:
	ScreenManager();
	~ScreenManager();
	
	UIScreen* GetScreen() const {return defaultScreen;};
	
protected slots:
	void OnSelectedScreenChanged(const HierarchyTreeScreenNode*);
	
private:
	UIScreen* defaultScreen;
	ScreenControl* activeScreenControl;
};

#endif /* defined(__UIEditor__ScreenManager__) */
