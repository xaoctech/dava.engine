package com.dava.framework;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import android.opengl.GLSurfaceView;
//import android.util.Log;
import android.util.Log;

public class JNIContextFactory implements GLSurfaceView.EGLContextFactory 
{
    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    
    private static int glVersion30 = 3;
    private static int glVersion20 = 2;
    
	public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) 
    {
        JNIConfigChooser.printConfig(egl, display, eglConfig);
        
        JNIConst.checkEglError("Before eglCreateContext", egl);
        
        EGLContext context = createOpenGLESContext(glVersion30, egl, display, eglConfig);
        if(context == null)
        {
        	Log.w(JNIConst.LOG_TAG, "[JNIContextFactory::createContext] OpenGLES 3.0 is not supported");
        	context = createOpenGLESContext(glVersion20, egl, display, eglConfig);
        }
        
        JNIConst.checkEglError("After eglCreateContext", egl);

        return context;
    }

	public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) 
    {
        egl.eglDestroyContext(display, context);
    }
	
	private EGLContext createOpenGLESContext(int openglESVersion, EGL10 egl, EGLDisplay display, EGLConfig eglConfig)
	{
        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, openglESVersion, EGL10.EGL_NONE };
        return egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
	}
}
