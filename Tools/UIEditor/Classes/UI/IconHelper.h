#ifndef __UIEDITOR__ICONHELPER__
#define __UIEDITOR__ICONHELPER__

#include <QString>
#include "DAVAEngine.h"

using namespace DAVA;

class IconHelper
{
public:
	static QString GetIconPathForClassName(const QString& className);
	static QString GetIconPathForUIControl(UIControl* uiControl);
};

#endif /* defined(__UIEDITOR__ICONHELPER__) */
