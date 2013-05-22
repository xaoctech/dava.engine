/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

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

	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	jmethodID mid = GetMethodID(javaClass, "ShowField", "(FFFFLjava/lang/String;)V");
	if (mid)
	{
		jstring jStrDefaultText = GetEnvironment()->NewStringUTF(defaultText);
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid, rect.x, rect.y, rect.dx, rect.dy, jStrDefaultText);
		GetEnvironment()->DeleteLocalRef(jStrDefaultText);
		activeTextField = textField;
		SafeRetain(activeTextField);
	}
	ReleaseJavaClass(javaClass);
}

void JniTextField::HideField()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return;

	jmethodID mid = GetMethodID(javaClass, "HideField", "()V");
	if (mid)
	{
		GetEnvironment()->CallStaticVoidMethod(javaClass, mid);
	}

	ReleaseJavaClass(javaClass);
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
	if (!activeTextField)
		return false;

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
