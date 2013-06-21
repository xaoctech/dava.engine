package com.dava.framework;

public class JNITextField {
	final static String TAG = "JNITextField";
	
	public static void ShowField(final float x, final float y, final float dx, final float dy, final String defaultText)
	{
		final JNIActivity activity = JNIActivity.GetActivity();
		activity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				activity.ShowEditText(x, y, dx, dy, defaultText);
			}
		});
	}
	
	public static void HideField()
	{
    	final JNIActivity activity = JNIActivity.GetActivity();
    	if (activity.IsEditTextVisible())
    	{
    		String text = activity.GetEditText();
    		activity.runOnUiThread(new Runnable() {
    			@Override
    			public void run() {
    				activity.HideEditText();
    			}
    		});
    		FieldHiddenWithText(text);
    	}
	}
	
	public static native void FieldHiddenWithText(String text);
	public static native void TextFieldShouldReturn();
	public static native boolean TextFieldKeyPressed(int replacementLocation, int replacementLength, String replacementString);
}
