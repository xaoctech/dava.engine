package com.dava.framework;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.MotionEvent;

public class JNIGLSurfaceView extends GLSurfaceView 
{
	private JNIRenderer mRenderer = null;

    private native void nativeOnTouch(int action, int id, float x, float y, long time);
    private native void nativeOnKeyUp(int keyCode);
    private native void nativeOnResumeView();
    private native void nativeOnPauseView();

    public JNIGLSurfaceView(Context context) 
    {
        super(context);
        Init();
    }
    
    public JNIGLSurfaceView(Context context, AttributeSet attrs)
    {
		super(context, attrs);
		Init();
	}

    private void Init()
    {
    	this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        
//        setPreserveEGLContextOnPause(true);
        setEGLContextFactory(new JNIContextFactory());
        setEGLConfigChooser(new JNIConfigChooser(8, 8, 8, 8, 16, 8));
        
        mRenderer = new JNIRenderer();
        setRenderer(mRenderer);
    }
    
    @Override
	public void onPause()
	{
    	queueEvent(new Runnable() 
    	{
    		public void run() 
    		{
    	    	nativeOnPauseView();
    		}
    	});
		super.onPause();
	}

    @Override
	public void onResume()
	{
		super.onResume();
		nativeOnResumeView();
	}
    
    class InputRunnable implements Runnable
    {
    	class InputEvent
    	{
    		int id;
    		float x;
    		float y;
    		
    		InputEvent(int id, float x, float y)
    		{
    			this.id = id;
    			this.x = x;
    			this.y = y;
    		}
    	}
    	
    	InputEvent[] events;
		long time;
		int action;
    	
    	public InputRunnable(final MotionEvent event)
    	{
    		time = event.getEventTime();
    		action = event.getActionMasked();
    		if (action == MotionEvent.ACTION_MOVE)
    		{
    			int pointerCount = event.getPointerCount();
    			events = new InputEvent[pointerCount];
    			for (int i = 0; i < pointerCount; ++i)
    			{
    				events[i] = new InputEvent(event.getPointerId(i), event.getX(i), event.getY(i));
    			}
    		}
    		else
    		{
    			int actionIdx = event.getActionIndex();
    			assert(actionIdx <= event.getPointerCount());
    			events = new InputEvent[1];
    			events[0] = new InputEvent(event.getPointerId(actionIdx), event.getX(actionIdx), event.getY(actionIdx));
    		}
    	}

		@Override
		public void run() {
			int size = events.length;
			for (int i = 0; i < size; ++i)
			{
				nativeOnTouch(action, events[i].id, events[i].x, events[i].y, time);
			}
		}
    	
    }

    public boolean onTouchEvent(final MotionEvent event) 
    {
    	queueEvent(new InputRunnable(event));
        return true;
    }
}
