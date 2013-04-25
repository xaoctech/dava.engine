#include "IconHelper.h"

QString IconHelper::GetIconPathForClassName(const QString &className)
{
	QString s = ":/Icons/" + className.toLower() + ".png";

	return s;
}

QString IconHelper::GetIconPathForUIControl(DAVA::UIControl *uiControl)
{
	QString className = "UIControl";
	if (!uiControl->GetCustomControlType().empty())
	{
		className = "UICustomControl";
	}
	else if (dynamic_cast<UIButton*>(uiControl))
	{
		className = "UIButton";
	}
	else if (dynamic_cast<UIList*>(uiControl))
	{
		className = "UIList";
	}
	else if (dynamic_cast<UISlider*>(uiControl))
	{
		className = "UISlider";
	}
	else if (dynamic_cast<UISpinner*>(uiControl))
	{
		className = "UISpinner";
	}
	else if (dynamic_cast<UIStaticText*>(uiControl))
	{
		className = "UIStaticText";
	}
	else if (dynamic_cast<UISwitch*>(uiControl))
	{
		className = "UISwitch";
	}
	else if (dynamic_cast<UITextField*>(uiControl))
	{
		className = "UITextField";
	}
	else if (dynamic_cast<UIAggregatorControl*>(uiControl))
	{
		className = "UIAggregatorControl";
	}

	return GetIconPathForClassName(className);
}