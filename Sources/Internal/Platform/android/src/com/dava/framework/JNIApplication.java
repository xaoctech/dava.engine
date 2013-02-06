package com.dava.framework;

import android.app.Application;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.util.Log;

public class JNIApplication extends Application
{
	private native void OnCreateApplication(String documentPath, String appPath, String logTag, String packageName); 
	private native void OnConfigurationChanged(); 
	private native void OnLowMemory(); 
	private native void OnTerminate(); 
	private native void SetAssetManager(AssetManager mngr);
	
	@Override
	public void onCreate()
	{
		super.onCreate();
		Log.i(JNIConst.LOG_TAG, "[Application::onCreate] start"); 
		
		String docDir = this.getExternalFilesDir(STORAGE_SERVICE).getAbsolutePath();
		
		String apkFilePath = null;
		ApplicationInfo appInfo = null;
		PackageManager packMgmr = getPackageManager();
		try 
		{
			appInfo = packMgmr.getApplicationInfo(JNIConst.PACKAGE_NAME, 0);
		} 
		catch (NameNotFoundException e) 
		{
			e.printStackTrace();
			throw new RuntimeException("Unable to locate assets, aborting...");
		}
		apkFilePath = appInfo.sourceDir;
		
		Log.w(JNIConst.LOG_TAG, String.format("[Application::onCreate] apkFilePath is %s", apkFilePath)); 
		OnCreateApplication(docDir, apkFilePath, JNIConst.LOG_TAG, JNIConst.PACKAGE_NAME);
		
		SetAssetManager(getAssets());

		Log.i(JNIConst.LOG_TAG, "[Application::onCreate] finish"); 
	}
	

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		Log.w(JNIConst.LOG_TAG, String.format("[Application::onConfigurationChanged]")); 

		super.onConfigurationChanged(newConfig);

		OnConfigurationChanged();
	}

	@Override
	public void onLowMemory()
	{
		Log.w(JNIConst.LOG_TAG, String.format("[Application::onLowMemory]")); 

		OnLowMemory();

		super.onLowMemory(); 
	}
	
	@Override
	public void onTerminate()
	{
    	Log.w(JNIConst.LOG_TAG, String.format("[Application::onTerminate]")); 

/*    	This method is for use in emulated process environments. 
 * 		It will never be called on a production Android device, 
 * 		where processes are removed by simply killing them; 
 * 		no user code (including this callback) is executed when doing so.
 */

		super.onTerminate();
	}
}

