package com.dava.framework;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import android.os.Build;

public class JNICrashReporter {
	
	public static String GetReport()
	{
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd_HH:mm:ss");
		String currentDateandTime = sdf.format(new Date());
		
		String report = new String();
		report += ("\nTime: " + currentDateandTime + "\n");
		report += ("DeveiceInfo:\n");
		report += ("MANUFACTURER: " + Build.MANUFACTURER + "\n");
		report += ("MODEL: " + Build.MODEL + "\n");
		report += ("OS: " + Build.VERSION.RELEASE + "\n");
		report += ("Locale: " + Locale.getDefault().getDisplayLanguage(Locale.US) + "\n");
		report += ("Country: " + JNIActivity.GetActivity().getResources().getConfiguration().locale.getCountry() + "\n");
		report += ("TimeZone: " + TimeZone.getDefault().getDisplayName(Locale.US) + "\n");
		report += ("\n");
		
		return report;
	}
	
	public static void ThrowJavaExpetion(String cppSignal) throws Exception
	{
		String report = GetReport();
		report += "\nCPP stack:\n";
		report += cppSignal;
		
		Thread.UncaughtExceptionHandler handler = Thread.currentThread().getUncaughtExceptionHandler();
		handler.uncaughtException(Thread.currentThread(), new Exception(report));
		report.wait();
	}
}
