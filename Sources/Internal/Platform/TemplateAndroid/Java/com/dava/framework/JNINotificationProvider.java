package com.dava.framework;

import android.R;
import android.app.Notification;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.content.res.AssetManager;


import android.app.NotificationManager;
import android.content.Context;
import android.support.v4.app.NotificationCompat;

public class JNINotificationProvider {
	private static NotificationCompat.Builder builder = null;
	private static NotificationManager notificationManager = null;
	private static AssetManager assetsManager = null;
	private static boolean isInited = false;
	
    static void Init() {
		Context context = JNIApplication.GetApplication();
		assetsManager = context.getAssets();
		notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		builder = new NotificationCompat.Builder(context);

		isInited = null != builder && null != notificationManager && null != assetsManager;
	}
    
    static void CleanBuilder()
    {
    	if (null != builder)
    	{
    		builder.setContentTitle("")
    			.setContentText("")
    			.setProgress(0, 0, false);
    	}
    }
	
	static void AttachToActivity()
	{
		JNIActivity.GetActivity().InitNotification(builder);
	}
	
    static void NotifyProgress(int id, String title, String text, int maxValue, int value) {
		if (isInited) {
			CleanBuilder();
			builder.setContentTitle(title)
				.setContentText(text)
				.setProgress(maxValue, value, false);
			notificationManager.notify(id, builder.build());
		}
	}
	
    static void NotifyText(int id, String title, String text) {

		if (isInited) {
			CleanBuilder();
			builder.setContentTitle(title)
					.setContentText(text);
			notificationManager.notify(id, builder.build());
		}
	}
    
    static void HideNotification(int id) {
		if (isInited){
			CleanBuilder();
			notificationManager.cancel(id);
		}
	}
    

}
