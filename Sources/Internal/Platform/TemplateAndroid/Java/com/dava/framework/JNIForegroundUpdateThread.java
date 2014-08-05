package com.dava.framework;

public class JNIForegroundUpdateThread {
	private Object needExit = null;
	private Thread thread = null;
	
	
	protected void StartBackgroundDownloadThread() {
		needExit = null;
		thread = new Thread(new Runnable() {
			
			@Override
			public void run() {
				RegisterForegroundThread();
				while (needExit == null)
				{
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					JNIDownloadManager.UpdateDownloadManager();
					JNINotification.UpdateNotification();
				}
				UnRegisterForegroundThread();
				
				Thread.yield();
				synchronized (needExit) {
					needExit.notify();
				}
				thread = null;
			}
		});
		thread.start();
	}
	
	protected void WaitExitDownloadTread() {
		if (thread != null) {
			needExit = new Object();
			synchronized (needExit) {
				try {
					needExit.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	native void RegisterForegroundThread();
	native void UnRegisterForegroundThread();
}
