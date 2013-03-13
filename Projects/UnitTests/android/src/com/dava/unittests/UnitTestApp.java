package com.dava.unittests;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;

import com.dava.framework.JNIApplication;

public class UnitTestApp extends JNIApplication {
	
	public static String PACKAGE_NAME = "com.dava.unittests";

	
	
	@Override
	public void onCreate() {
		super.onCreate();
		
		ApplicationInfo appInfo = null;
		PackageManager packMgmr = getPackageManager();
		try 
		{
			appInfo = packMgmr.getApplicationInfo(PACKAGE_NAME, 0);
		} 
		catch (NameNotFoundException e) 
		{
			e.printStackTrace();
			throw new RuntimeException("Unable to locate assets, aborting...");
		}
		
		CreateApplication(appInfo.sourceDir);
	}
	
	static {
		System.loadLibrary("UnitTestsLib");
		
		try {
			//Thread.sleep(10000);
			Thread.sleep(0);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}
