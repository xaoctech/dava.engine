#ifndef __UICHEKBOX_H__
#define __UICHEKBOX_H__

#include "DAVAEngine.h"

using namespace DAVA;

class UICheckBoxDelegate
{
public:
    
    virtual void ValueChanged(bool newValue) = 0;
};

class UICheckBox : public UIControl
{
public:

	UICheckBox();
	UICheckBox(const String &spriteName, const Rect &rect, bool rectInAbsoluteCoordinates = false);

	virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
    virtual void LoadFromYamlNodeCompleted();

	void SetChecked(bool _checked, bool needDelegateCall);
	bool Checked();

    void SetDelegate(UICheckBoxDelegate *delegate);
    
private:

	void OnClick(BaseObject * owner, void * userData, void * callerData);

	bool checked;
    UICheckBoxDelegate *checkboxDelegate;
};

#endif //#ifndef __UICHEKBOX_H__
