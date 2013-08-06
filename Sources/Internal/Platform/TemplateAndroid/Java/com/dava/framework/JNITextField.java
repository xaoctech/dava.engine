package com.dava.framework;

import java.util.concurrent.Callable;
import java.util.concurrent.FutureTask;


public class JNITextField {
	final static String TAG = "JNITextField";
	
	public static void ShowField(final float x, final float y, final float dx, final float dy, final String defaultText)
	{
		final FutureTask<Void> task = new FutureTask<Void> (new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				JNIActivity.GetActivity().ShowEditText(x, y, dx, dy, defaultText);
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				task.run();
			}
		});
		
		while (!task.isDone())
			Thread.yield();
		TextFieldKeyPressed(0, 0, defaultText);
	}
	
	static String text;
	public static void HideField()
	{
		final FutureTask<Void> task = new FutureTask<Void> (new Callable<Void>() {
			@Override
			public Void call() throws Exception {
				JNIActivity activity = JNIActivity.GetActivity();
				text = activity.GetEditText();
				activity.HideEditText();
				return null;
			}
		});
		
		JNIActivity.GetActivity().runOnUiThread(new Runnable() {
			@Override
			public void run() {
				task.run();
			}
		});
		
		while (!task.isDone())
			Thread.yield();
		
		FieldHiddenWithText(text);
	}
	
	public static native void FieldHiddenWithText(String text);
	public static native void TextFieldShouldReturn();
	public static native boolean TextFieldKeyPressed(int replacementLocation, int replacementLength, String replacementString);
}
