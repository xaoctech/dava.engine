package com.dava.framework;

import java.util.Locale;
import java.util.TimeZone;

import android.os.Build;

public class JNIDeviceInfo {
	final static String TAG = "JNIDeviceInfo";

	public static void GetVersion()
	{
		SetJString(Build.VERSION.RELEASE);
	}

	public static void GetManufacturer()
	{
		SetJString(Build.MANUFACTURER);
	}

	public static void GetModel()
	{
		SetJString(Build.MODEL);
	}

	public static void GetLocale()
	{
		SetJString(Locale.getDefault().getDisplayLanguage(Locale.US));
	}

	public static void GetRegion()
	{
		String country = JNIActivity.GetActivity().getResources().getConfiguration().locale.getCountry();
		SetJString(country);
	}

	public static void GetTimeZone()
	{
		SetJString(TimeZone.getDefault().getDisplayName(Locale.US));
	}

	public static native void SetJString(String str);
}
