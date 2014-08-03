package com.dava.framework;

public class JNIDownloadManager {
	
	private Object needExit = null;
	private Thread thread = null;
	
	
	protected void StartBackgroundDownloadThread() {
		needExit = null;
		thread = new Thread(new Runnable() {
			
			@Override
			public void run() {
				while (needExit == null)
				{
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					UpdateDownloadManager();
				}
				
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
	
	protected static native void UpdateDownloadManager();
}
