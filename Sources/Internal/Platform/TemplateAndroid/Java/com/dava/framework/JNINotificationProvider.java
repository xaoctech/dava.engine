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
	
	static void AttachToActivity()
	{
		JNIActivity.GetActivity().InitNotification(builder);
	}
	
    static void NotifyProgress(
			int id,
			String title, 
			String text, 
			int maxValue, 
			int value) {
		if (isInited) {
			builder.setContentTitle(title)
				.setContentText(text)
				.setProgress(maxValue, value, false);
			notificationManager.notify(id, builder.build());
		}
	}
	
    static void HideNotification(int id) {
		if (isInited)
			notificationManager.cancel(id);
	}
    
    static void SetNotificationTitle(int id, String title) {
    	if (isInited) {
    		Notification notificationToSet = builder
   		         .setContentTitle(title)
   		         .build();
   			    
   			notificationManager.notify(id, notificationToSet);
    	}
    }
    
    static void SetNotificationText(int id, String text) {
    	if (isInited) {
    		Notification notificationToSet = builder
   		         .setContentText(text)
   		         .build();
   			    
   			notificationManager.notify(id, notificationToSet);
    	}
    }
	
    static void SetNotificationProgress(int id, int total, int current)
    {
    	if (isInited) {
    		Notification notificationToSet = builder
   		         .setProgress(total, current, false)
   		         .build();
   			    
   			notificationManager.notify(id, notificationToSet);
    	}
    }
}
