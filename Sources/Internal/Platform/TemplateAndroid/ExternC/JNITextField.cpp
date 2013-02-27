#include "AndroidLayer.h"
#include "UI/UITextFieldAndroid.h"

char text[256] = {0};

extern "C"
{
	void Java_com_dava_framework_JNITextField_FieldHiddenWithText(JNIEnv* env, jobject classthis, jstring jStrText)
	{
		if (DAVA::JniTextField::jniTextField)
		{
			CreateStringFromJni(env, jStrText, text);
			DAVA::JniTextField::jniTextField->FieldHiddenWithText(text);
		}
	}


	void Java_com_dava_framework_JNITextField_TextFieldShouldReturn(JNIEnv* env, jobject classthis)
	{
		if (DAVA::JniTextField::jniTextField)
		{
			DAVA::JniTextField::jniTextField->TextFieldShouldReturn();
		}
	}

	bool Java_com_dava_framework_JNITextField_TextFieldKeyPressed(JNIEnv* env, jobject classthis, int replacementLocation, int replacementLength, jstring replacementString)
	{
		if (DAVA::JniTextField::jniTextField)
		{
			CreateStringFromJni(env, replacementString, text);
			return DAVA::JniTextField::jniTextField->TextFieldKeyPressed(replacementLocation, replacementLength, text);
		}
		return false;
	}
};
