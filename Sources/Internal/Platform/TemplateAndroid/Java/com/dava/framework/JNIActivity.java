package com.dava.framework;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.text.InputFilter;
import android.text.Spanned;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;

public abstract class JNIActivity extends Activity implements JNIAccelerometer.JNIAccelerometerListener
{
	private static int errorState = 0;

	private JNIAccelerometer accelerometer = null;
	private GLSurfaceView glView = null;
	private EditText editText = null;
    
    private native void nativeOnCreate(boolean isFirstRun);
    private native void nativeOnStart();
    private native void nativeOnStop();
    private native void nativeOnDestroy();
    private native void nativeIsFinishing();
    private native void nativeOnAccelerometer(float x, float y, float z);
    
    private boolean isFirstRun = true;
    
    public abstract JNIGLSurfaceView GetSurfaceView();
    
    private static JNIActivity activity = null;
    
    public static JNIActivity GetActivity()
	{
		return activity;
	}
    
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
    	activity = this;
        super.onCreate(savedInstanceState);
        
        if(null != savedInstanceState)
        {
        	isFirstRun = savedInstanceState.getBoolean("isFirstRun");
        }
        
    	// The activity is being created.
        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate]");

        /*requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        super.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        super.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);*/

        // initialize accelerometer
        SensorManager sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        accelerometer = new JNIAccelerometer(this, sensorManager);

        // initialize GL VIEW
        glView = GetSurfaceView();
        assert(glView != null);
        glView.setFocusableInTouchMode(true);
        glView.setClickable(true);
        glView.setFocusable(true);
        glView.requestFocus();
        
        editText = new EditText(this);
        InitEditText(editText);
    
        if(0 != errorState)
        {

        }

        Log.i(JNIConst.LOG_TAG, "[Activity::onCreate] isFirstRun is " + isFirstRun); 
        nativeOnCreate(isFirstRun);
    }
    
    @Override
    protected void onStart()
    {
    	super.onStart();
    	// The activity is about to become visible.
    	
        Log.i(JNIConst.LOG_TAG, "[Activity::onStart]");
        
        //editText.setVisibility(EditText.INVISIBLE);
        
        //call native method
        nativeOnStart();
    }
    
    @Override
    protected void onRestart()
    {
    	super.onRestart();
    	// The activity is about to become visible.
    	
        Log.i(JNIConst.LOG_TAG, "[Activity::onRestart]");
        
        //TODO: VK: may be usefull
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onSaveInstanceState]");

        outState.putBoolean("isFirstRun", isFirstRun);
    	
    	super.onSaveInstanceState(outState);
    }
    
    @Override
    protected void onResume() 
    {
        super.onResume();
        // The activity has become visible (it is now "resumed").
		Log.i(JNIConst.LOG_TAG, "[Activity::onResume] start");

        // activate accelerometer
        if(null != accelerometer)
        {
        	if(accelerometer.IsSupported())
        	{
        		accelerometer.Start();
        	}
        }
        
        // activate GLView 
        if(null != glView)
        {
        	glView.onResume();
        }

        Log.i(JNIConst.LOG_TAG, "[Activity::onResume] finish");
    }

    
    @Override
    protected void onPause() 
    {
        super.onPause();
        // Another activity is taking focus (this activity is about to be "paused").

        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] start");

        // deactivate accelerometer
        if(null != accelerometer)
        {
        	if(accelerometer.IsActive())
        	{
        		accelerometer.Stop();
        	}
        }

        // deactivate GLView 
        if(null != glView)
        {
        	glView.onPause();
        }
        
        boolean isActivityFinishing = isFinishing();
        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] isActivityFinishing is " + isActivityFinishing);
        if(isActivityFinishing)
        {
        	nativeIsFinishing();
        }
        
        Log.i(JNIConst.LOG_TAG, "[Activity::onPause] finish");
    }

    
    @Override
    protected void onStop()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] start");
        
        //call native method
        nativeOnStop();

        Log.i(JNIConst.LOG_TAG, "[Activity::onStop] finish");
        
        super.onStop();
    	// The activity is no longer visible (it is now "stopped")
    }
    
    
    @Override
    protected void onDestroy()
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] start");

        //call native method
        nativeOnDestroy();

        Log.i(JNIConst.LOG_TAG, "[Activity::onDestroy] finish");

        super.onDestroy();
    	// The activity is about to be destroyed.
    }

    @Override
    protected void onPostResume() 
    {
        Log.i(JNIConst.LOG_TAG, "[Activity::onPostResume] ****");
        
    	super.onPostResume();
    }
    
    
    public void onAccelerationChanged(float x, float y, float z)
	{
		nativeOnAccelerometer(x, y, z);
	}
    
    static boolean inputFilterRes = false;
    private void InitEditText(EditText editText)
    {
    	FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(new FrameLayout.MarginLayoutParams(0, 0));
        params.leftMargin = -1;
        params.topMargin = -1;
        addContentView(editText, params);

        editText.setMaxLines(1);
        editText.setBackgroundColor(Color.BLACK);
        editText.setTextColor(Color.WHITE);
        
        InputFilter inputFilter = new InputFilter() {
			
			@Override
			public CharSequence filter(final CharSequence source, final int start, final int end,
					Spanned dest, int dstart, int dend) {
				if (source.length() > 1)
					return source;
				
				inputFilterRes = false;
				final Object mutex = new Object();
				glView.queueEvent(new Runnable() {
					public void run() {
						inputFilterRes = JNITextField.TextFieldKeyPressed(start, end - start, source.toString());
						synchronized (mutex) {
							mutex.notify();
						}
					}
				});
				synchronized (mutex) {
					try {
						mutex.wait();
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}
				if (inputFilterRes)
					return source;
				return "";
			}
		};
        editText.setFilters(new InputFilter[]{inputFilter});

        editText.setOnKeyListener(new View.OnKeyListener() {
			
			@Override
			public boolean onKey(View v, int keyCode, KeyEvent event) {
				if (event.getAction() == KeyEvent.ACTION_DOWN)
				{
					if(keyCode == KeyEvent.KEYCODE_ENTER)
					{
						glView.queueEvent(new Runnable() {
							@Override
							public void run() {
								JNITextField.TextFieldShouldReturn();
							}
						});
					}
					else
					{
						glView.onKeyDown(keyCode, event);
					}
					return true;
				}
				return false;
			}
		});
        
        editText.setOnGenericMotionListener(new View.OnGenericMotionListener() {
			
			@Override
			public boolean onGenericMotion(View v, MotionEvent event) {
				return glView.onGenericMotionEvent(event);
			}
		});
    }
	
	public void ShowEditText(float x, float y, float dx, float dy, String defaultText)
	{
		editText.setText(defaultText);
		editText.setSelection(editText.getText().length());
		
		//TODO: YZ fix incorrect control height
		dy += 5f;	
		//dy += editText.getPaddingBottom() - editText.getPaddingTop();
		
		FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) editText.getLayoutParams();
		params.leftMargin = (int)x;
		params.topMargin = (int)y;
		params.width = (int)(dx + 0.5f);
		params.height = (int)(dy + 0.5f); 
		editText.setLayoutParams(params);
		
		editText.requestFocus();
		InputMethodManager input = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		input.showSoftInput(editText, InputMethodManager.SHOW_IMPLICIT);
	}
	
	public void HideEditText(boolean notifyCore)
	{
		InputMethodManager input = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		input.hideSoftInputFromWindow(editText.getWindowToken(), 0);
		
		FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) editText.getLayoutParams();
		params.leftMargin = (int)-1;
		params.topMargin = (int)-1;
		params.width = (int)0;
		params.height = (int)0;
		editText.setLayoutParams(params);
		if (notifyCore)
		{
			final String text = editText.getText().toString();
			glView.queueEvent(new Runnable() {
				@Override
				public void run() {
					JNITextField.FieldHiddenWithText(text);
				}
			});
		}
	}
	
	public String GetEditText()
	{
		return editText.getText().toString();
	}
}
