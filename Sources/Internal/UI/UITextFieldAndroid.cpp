#include "UITextFieldAndroid.h"

using namespace DAVA;

JniTextField* JniTextField::jniTextField = NULL;

JniTextField::JniTextField() :
	JniExtension("com/dava/framework/JNITextField")
{
	activeTextField = NULL;
}

void JniTextField::ShowField(UITextField* textField, const Rect& controlRect, const char* defaultText)
{
	Rect rect = V2P(controlRect);

	if (activeTextField)
		HideField();

	jmethodID mid = GetMethodID("ShowField", "(FFFFLjava/lang/String;)V");
	if (mid)
	{
		jstring jStrDefaultText = GetEnvironment()->NewStringUTF(defaultText);
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, rect.x, rect.y, rect.dx, rect.dy, jStrDefaultText);
		GetEnvironment()->DeleteLocalRef(jStrDefaultText);
		activeTextField = textField;
		SafeRetain(activeTextField);
	}
}

void JniTextField::HideField()
{
	jmethodID mid = GetMethodID("HideField", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid);
	}

	// SafeRelease(activeTextField);
}

void JniTextField::FieldHiddenWithText(const char* text)
{
	if (!activeTextField)
		return;

	UITextField* _activeTextField = activeTextField;
	activeTextField = NULL;

	WideString strText = StringToWString(text);
	_activeTextField->SetText(strText);

	UIControl* curentActiveControl = UIControlSystem::Instance()->GetFocusedControl();
	if (curentActiveControl == _activeTextField)
	{
		UIControlSystem::Instance()->SetFocusedControl(_activeTextField->GetParent(), true);
	}
	SafeRelease(_activeTextField);
}

void JniTextField::TextFieldShouldReturn()
{
	DVASSERT(activeTextField);
	UITextFieldDelegate* delegate = activeTextField->GetDelegate();
	if (delegate)
		delegate->TextFieldShouldReturn(activeTextField);
}

bool JniTextField::TextFieldKeyPressed(int32 replacementLocation, int32 replacementLength, const char* text)
{
	DVASSERT(activeTextField);
	UITextFieldDelegate* delegate = activeTextField->GetDelegate();
	if (!delegate)
		return true;

	WideString strText = StringToWString(text);
	return delegate->TextFieldKeyPressed(activeTextField, replacementLocation, replacementLength, strText);
}

UITextFieldAndroid::UITextFieldAndroid(UITextField* textField)
{
	this->textField = textField;
	if (!JniTextField::jniTextField)
	{
		JniTextField::jniTextField = new JniTextField();
	}
}

UITextFieldAndroid::~UITextFieldAndroid()
{

}

void UITextFieldAndroid::ShowField()
{
	String text = WStringToString(textField->GetText());
	JniTextField::jniTextField->ShowField(textField, textField->GetRect(), text.c_str());
}

void UITextFieldAndroid::HideField()
{
	JniTextField::jniTextField->HideField();
}


//TextFieldKeyPressed
//TextFieldShouldReturn
