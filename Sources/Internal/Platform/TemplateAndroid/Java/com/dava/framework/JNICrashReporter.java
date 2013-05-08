package com.dava.framework;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;

import android.os.Build;

public class JNICrashReporter {
	
	public static String GetReportFile()
	{
		JNIApplication app = JNIApplication.GetApplication();
		String docDir = app.GetDocumentPath();
		
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd_HHmmss");
		String currentDateandTime = sdf.format(new Date());
		
		String fileName = String.format("/CrashReport_%s.txt", currentDateandTime);
		File file = new File(docDir + fileName);
		PrintWriter writer;
		try {
			writer = new PrintWriter(file);
			
			writer.write("DeveiceInfo:\n");
			writer.write("MANUFACTURER: " + Build.MANUFACTURER + "\n");
			writer.write("MODEL: " + Build.MODEL + "\n");
			writer.write("OS: " + Build.VERSION.RELEASE + "\n");
			writer.write("Locale: " + Locale.getDefault().getDisplayLanguage(Locale.US) + "\n");
			writer.write("Country: " + JNIActivity.GetActivity().getResources().getConfiguration().locale.getCountry() + "\n");
			writer.write("TimeZone: " + TimeZone.getDefault().getDisplayName(Locale.US) + "\n");
			writer.write("\n");
			
			writer.write("JavaCallStack:\n");
			Map<Thread, StackTraceElement[]> callStacks = Thread.getAllStackTraces();
			for (Map.Entry<Thread, StackTraceElement[]> item : callStacks.entrySet()) {
				Thread thread = item.getKey();
				writer.write(String.format("Thread name=%s, id=%s\n", thread.getName(), thread.getId()));
				StackTraceElement[] stack = item.getValue();
				for (StackTraceElement stackTraceElement : stack) {
					writer.write(String.format("%s, %s, line=%d\n", 
							stackTraceElement.getClassName(), 
							stackTraceElement.getMethodName(), 
							stackTraceElement.getLineNumber()));
				}
			}
			writer.write("\n");
			
			writer.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}

		return file.getAbsolutePath();
	}
}
