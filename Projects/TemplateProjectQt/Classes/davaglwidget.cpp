#include "DAVAEngine.h"

#include "davaglwidget.h"
#include "ui_davaglwidget.h"

#include "QResizeEvent"

#if defined (__DAVAENGINE_MACOS__)
	#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
	#include "Platform/Qt/Win32/QtLayerWin32.h"
	#include "Platform/Qt/Win32/CorePlatformWin32.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

DavaGLWidget::DavaGLWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DavaGLWidget)
{
	ui->setupUi(this);
	
	isFirstDraw = true;

#if defined (__DAVAENGINE_MACOS__)
	DAVA::QtLayerMacOS *qtLayer = new DAVA::QtLayerMacOS();
    qtLayer->InitializeGlWindow((void *)this->winId(), this->size().width(), this->size().height());
    
#elif defined (__DAVAENGINE_WIN32__)
	DAVA::QtLayerWin32 *qtLayer = new DAVA::QtLayerWin32();
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	qtLayer->SetWindow(hInstance, this->winId(), this->size().width(), this->size().height());
	qtLayer->OnResume();
	
#else 
	DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    counter = 0;
    divider = 0.5f;
    
	startTimer(20);
    
	DAVA::QtLayer::Instance()->Resize(size().width(), size().height());
}

DavaGLWidget::~DavaGLWidget()
{
    DAVA::QtLayer::Instance()->Release();

    delete ui;
}


void DavaGLWidget::paintEvent(QPaintEvent *)
{
    //Do nothing
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{	
    QWidget::resizeEvent(e);
        
	DAVA::QtLayer::Instance()->Resize(e->size().width(), e->size().height());
}

void DavaGLWidget::timerEvent(QTimerEvent *e)
{
//    QWidget::timerEvent(e);
    
//    ++counter;
//    if(400 == counter)
//    {
//        counter = 0;
//        
//        QSize sz = this->size();
//        sz /= divider;
//        
//        divider = 1.f / divider;
//        
//        resize(sz);
//        
////        resize(this->size().height(), this->size().width());
//    }
    
    
	DAVA::QtLayer::Instance()->ProcessFrame();
}



void DavaGLWidget::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    
	DAVA::QtLayer::Instance()->OnResume();
}

void DavaGLWidget::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    
	DAVA::QtLayer::Instance()->OnSuspend();
}

#if defined(Q_WS_WIN)
bool DavaGLWidget::winEvent(MSG *message, long *result)
{
	DAVA::CoreWin32Platform *core = dynamic_cast<DAVA::CoreWin32Platform *>(DAVA::CoreWin32Platform::Instance());
	if (NULL != core)
	{
		return core->WinEvent(message, result);
	}

	return false;
}
#endif //#if defined(Q_WS_WIN)
