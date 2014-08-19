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
	
	JNINotificationProvider(Context context) {
		
		assetsManager = context.getAssets();
	}
	
	static void Init() {
		Context context = JNIApplication.GetApplication();
		notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
		builder = new NotificationCompat.Builder(context);
		JNIActivity.GetActivity().InitNotification(builder);
		
		isInited = null != builder && null != notificationManager && null != assetsManager;
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
    
	static void Notify()
	{
		if (isInited)
		{		
			   // Bitmap bmp=BitmapFactory.decodeStream(assetsManager.open("Data/1.jpg"));

			    Notification noti = builder
		         .setContentTitle("Title")
		         .setContentText("text \n next line dlgfkj ;sdlkfgj ;dkfjg; lsdjfg;lksdj f;gls d")
		         //.setSubText("SubText")
		         .setTicker("New BlitzNotification")
		         .setSmallIcon(R.drawable.btn_star_big_on)
		       //  .setLargeIcon(bmp)
		         .setProgress(100, 50, false)
		         .build();
			    
			    notificationManager.notify(1, noti);

			
			
		}
	}
}
