#ifndef __DAVAENGINE_UI_TEXT_FIELD_ANDROID_H__
#define __DAVAENGINE_UI_TEXT_FIELD_ANDROID_H__

#include "UI/UITextField.h"
#include "JniExtensions.h"

namespace DAVA
{

class JniTextField: public JniExtension
{
public:
	JniTextField();
	void ShowField(UITextField* textField, const Rect& rect, const char* defaultText);
	void HideField();
	void FieldHiddenWithText(const char* text);
	void TextFieldShouldReturn();
	bool TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const char* text);
public:
	static JniTextField* jniTextField;

private:
	UITextField* activeTextField;
};

class UITextFieldAndroid
{
public:
	UITextFieldAndroid(UITextField* textField);
	virtual ~UITextFieldAndroid();

	void ShowField();
	void HideField();

private:
	UITextField* textField;
};
};

#endif //__DAVAENGINE_UI_TEXT_FIELD_ANDROID_H__
