package com.dava.framework;

import java.util.ArrayList;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class JNIGLSurfaceView extends GLSurfaceView 
{
	private JNIRenderer mRenderer = null;

    private native void nativeOnInput(int action, int id, float x, float y, long time, int source);
    private native void nativeOnKeyDown(int keyCode);
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
    		int source;
    		
    		InputEvent(int id, float x, float y, int source)
    		{
    			this.id = id;
    			this.x = x;
    			this.y = y;
    			this.source = source;
    		}
    	}
    	
    	ArrayList<InputEvent> events;
		long time;
		int action;
		
    	public InputRunnable(final MotionEvent event)
    	{
    		time = event.getEventTime();
    		events = new ArrayList<InputEvent>();
    		action = event.getActionMasked();
    		if(action == MotionEvent.ACTION_MOVE)
    		{
        		int pointerCount = event.getPointerCount();
	    		for (int i = 0; i < pointerCount; ++i)
	    		{
	    			if((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) > 0)
	    			{
	    				events.add(new InputEvent(event.getPointerId(i), event.getX(i), event.getY(i), event.getSource()));
	    			}
	    			if((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
	    			{
	    				//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
	        			events.add(new InputEvent(0, event.getAxisValue(MotionEvent.AXIS_X, i), 0, event.getSource()));
	        			events.add(new InputEvent(1, event.getAxisValue(MotionEvent.AXIS_Y, i), 0, event.getSource()));
	        			events.add(new InputEvent(2, event.getAxisValue(MotionEvent.AXIS_Z, i), 0, event.getSource()));
	        			events.add(new InputEvent(3, event.getAxisValue(MotionEvent.AXIS_RX, i), 0, event.getSource()));
	        			events.add(new InputEvent(4, event.getAxisValue(MotionEvent.AXIS_RY, i), 0, event.getSource()));
	        			events.add(new InputEvent(5, event.getAxisValue(MotionEvent.AXIS_RZ, i), 0, event.getSource()));
	        			events.add(new InputEvent(6, event.getAxisValue(MotionEvent.AXIS_LTRIGGER, i), 0, event.getSource()));
	        			events.add(new InputEvent(7, event.getAxisValue(MotionEvent.AXIS_RTRIGGER, i), 0, event.getSource()));
	    			}
	    		}
    		}
    		else
    		{
    			int actionIdx = event.getActionIndex();
    			assert(actionIdx <= event.getPointerCount());
    			events.add(new InputEvent(event.getPointerId(actionIdx), event.getX(actionIdx), event.getY(actionIdx), event.getSource()));
    		}
    	}

		@Override
		public void run()
		{
			for (int i = 0; i < events.size(); ++i)
			{
				nativeOnInput(action, events.get(i).id, events.get(i).x, events.get(i).y, time, events.get(i).source);
			}
		}
    }

    class KeyInputRunnable implements Runnable
    {
    	int keyCode;
    	public KeyInputRunnable(int keyCode)
    	{
    		this.keyCode = keyCode;
    	}
    	
    	@Override
    	public void run() {
    		nativeOnKeyDown(keyCode);
    	}
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	queueEvent(new KeyInputRunnable(keyCode));
    	return true;
    }
    
    @Override
    public boolean onTouchEvent(final MotionEvent event) 
    {
    	queueEvent(new InputRunnable(event));
        return true;
    }
    
    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
    	queueEvent(new InputRunnable(event));
    	return true;
    }
}
