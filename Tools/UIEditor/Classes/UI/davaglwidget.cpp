#include "DAVAEngine.h"

#include "davaglwidget.h"
#include "ui_davaglwidget.h"

#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include "controllist.h"
#include "ScreenWrapper.h"
#include "HierarchyTreeController.h"
#include "ItemsCommand.h"
#include "CommandsController.h"

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
    
	DAVA::QtLayer::Instance()->Resize(size().width(), size().height());
    
    fpsTimer = new QTimer();
    connect(fpsTimer, SIGNAL(timeout()), this, SLOT(FpsTimerDone()));

    DAVA::RenderManager::Instance()->SetFPS(60);
    frameTime = 1000 / DAVA::RenderManager::Instance()->GetFPS();
    
    fpsTimer->start(frameTime);
	
	this->setAcceptDrops(true);	
	setMouseTracking(true);

	ScreenWrapper::Instance()->SetQtScreen(this);
}

DavaGLWidget::~DavaGLWidget()
{
    DAVA::QtLayer::Instance()->Release();

	DAVA::SafeDelete(fpsTimer);
    delete ui;
}


void DavaGLWidget::paintEvent(QPaintEvent *)
{
    //Do nothing
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{	
    QWidget::resizeEvent(e);

	DAVA::QtLayer::Instance()->Resize((int32)e->size().width(), (int32)e->size().height());
	
	//YZ fix load resource
	Core::Instance()->RegisterAvailableResourceSize((int32)e->size().width(), (int32)e->size().height(), "Gfx");
}

void DavaGLWidget::FpsTimerDone()
{
    QElapsedTimer timer;
    timer.start();
    
	DAVA::QtLayer::Instance()->ProcessFrame();
    
    int timeForNewFrame = frameTime - timer.elapsed();
    if(timeForNewFrame < 0)
    {
        fpsTimer->start(0);
    }
    else 
    {
        fpsTimer->start(timeForNewFrame);
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
		return core->WinEvent(message, result);
	}

	return false;
}
#endif //#if defined(Q_WS_WIN)

void DavaGLWidget::dropEvent(QDropEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (!controlData)
		return;
	
	CreateControlCommand* cmd = new CreateControlCommand(controlData->GetControlName(), event->pos());
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void DavaGLWidget::dragMoveEvent(QDragMoveEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (controlData && HierarchyTreeController::Instance()->GetActiveScreen())
	{
		event->accept();
		ScreenWrapper::Instance()->BacklightControl(event->pos());
	}
}

void DavaGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
	const QMimeData* data = event->mimeData();
	const ControlMimeData* controlData = dynamic_cast<const ControlMimeData*>(data);
	if (controlData && HierarchyTreeController::Instance()->GetActiveScreen())
		event->acceptProposedAction();
}

void DavaGLWidget::mouseMoveEvent(QMouseEvent * event)
{
	QWidget::mouseMoveEvent(event);
	
	if (event->type() == QEvent::MouseMove)
	{
		setCursor(ScreenWrapper::Instance()->GetCursorType(event->pos()));
		ScreenWrapper::Instance()->CursorMove(event->pos());
	}
}

void DavaGLWidget::wheelEvent(QWheelEvent *e)
{
	QWidget::wheelEvent(e);

	if (e->modifiers() & Qt::ControlModifier)
	{
		float delta = e->delta() * 0.001;
		ScreenWrapper::Instance()->UpdateScale(delta);
	}
}

void DavaGLWidget::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
    
    DAVA::QtLayer::Instance()->LockKeyboardInput(true);
}

void DavaGLWidget::focusOutEvent(QFocusEvent *e)
{
    QWidget::focusOutEvent(e);
	
    DAVA::QtLayer::Instance()->LockKeyboardInput(false);
}

void DavaGLWidget::keyPressEvent(QKeyEvent *)
{
	qDebug("DavaGLWidget::keyPressEvent");
}

void DavaGLWidget::keyReleaseEvent(QKeyEvent *)
{
	qDebug("DavaGLWidget::keyReleaseEvent");
}
