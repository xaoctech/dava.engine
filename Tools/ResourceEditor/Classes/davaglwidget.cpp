#include "DAVAEngine.h"

#include "davaglwidget.h"
#include "ui_davaglwidget.h"

#include <QApplication>
#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>

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
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

#if defined (__DAVAENGINE_MACOS__)
    setMouseTracking(true);
    
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

	DAVA::QtLayer::Instance()->SetDelegate(this);
	DAVA::QtLayer::Instance()->Resize(size().width(), size().height());

	willClose = false;

	DisableWidgetBlinking();
	InitFrameTimer();
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
    
    QPoint newPosition = mapTo(parentWidget(), QPoint(0, 0));
	DAVA::QtLayer::Instance()->Move(newPosition.x(), newPosition.y());
}

void DavaGLWidget::moveEvent(QMoveEvent *e)
{
	QWidget::moveEvent(e);

    QPoint newPosition = mapTo(parentWidget(), QPoint(0, 0));
	DAVA::QtLayer::Instance()->Move(newPosition.x(), newPosition.y());
}


void DavaGLWidget::FpsTimerDone()
{
    QElapsedTimer timer;
    timer.start();
    
	DAVA::QtLayer::Instance()->ProcessFrame();
    
	if(!willClose)
	{
		int timeForNewFrame = frameTime - timer.elapsed();
		if(timeForNewFrame < 0)
		{
			QTimer::singleShot(1, this, SLOT(FpsTimerDone()));
		}
		else 
		{
			QTimer::singleShot(timeForNewFrame, this, SLOT(FpsTimerDone()));
		}
	}
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
		return 
			core->WinEvent(message, result);
	}

	return false;
}
#endif //#if defined(Q_WS_WIN)

void DavaGLWidget::closeEvent(QCloseEvent *e)
{
	willClose = true;

	QWidget::closeEvent(e);
}

void DavaGLWidget::Quit()
{
    QApplication::quit();
}

QPaintEngine *DavaGLWidget::paintEngine() const
{
	return NULL;
}

void DavaGLWidget::DisableWidgetBlinking()
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
}

void DavaGLWidget::InitFrameTimer()
{
	DAVA::RenderManager::Instance()->SetFPS(60);
	frameTime = 1000 / DAVA::RenderManager::Instance()->GetFPS();
	QTimer::singleShot(frameTime, this, SLOT(FpsTimerDone()));
}

#if defined (Q_WS_MAC)
void DavaGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    DAVA::QtLayerMacOS *qtLayer = dynamic_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
    if(qtLayer)
    {
        const QRect geometry = this->geometry();
        qtLayer->MouseMoved(e->x() + geometry.x(), -e->y() - geometry.y());
    }
    
    QWidget::mouseMoveEvent(e);
}
#endif //#if defined (Q_WS_MAC)

