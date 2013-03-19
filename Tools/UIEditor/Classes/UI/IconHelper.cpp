#include "IconHelper.h"

QString IconHelper::GetIconPathForClassName(const QString &className)
{
	QString s = ":/Icons/" + className.toLower() + ".png";

	return s;
}

QString IconHelper::GetIconPathForUIControl(DAVA::UIControl *uiControl)
{
	QString className = "UIControl";
	if (dynamic_cast<UIButton*>(uiControl))
	{
		className = "UIButton";
	}
	else if (dynamic_cast<UIList*>(uiControl))
	{
		className = "UIList";
	}
	else if (dynamic_cast<UIScrollBar*>(uiControl))
	{
		className = "UIScrollBar";
	}
	else if (dynamic_cast<UISpinner*>(uiControl))
	{
		className = "UISpinner";
	}
	else if (dynamic_cast<UIStaticText*>(uiControl))
	{
		className = "UIStaticText";
	}
	else if (dynamic_cast<UIAggregatorControl*>(uiControl))
	{
		className = "UIAggregatorControl";
	}

	return GetIconPathForClassName(className);
}