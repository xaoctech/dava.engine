#include "Engine/Private/Qt/RenderWidgetOGL.h"

#if defined(__DAVAENGINE_QT__)
#include "Base/StaticSingleton.h"
#include "Base/TemplateHelpers.h"
#include "Engine/Private/Qt/RenderWidgetBackend.h"
#include "Concurrency/Thread.h"

#include <QWindow>
#include <QHBoxLayout>
#include <QThread>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QApplication>

namespace DAVA
{
class RenderWidgetOGL::RenderSurface : public QWindow
{
public:
    using Callback = DAVA::Function<void()>;
    RenderSurface(const Callback& created, const Callback& frame, const Callback& destroyed)
        : createCallback(created)
        , frameCallback(frame)
        , destroyCallback(destroyed)
    {
        setSurfaceType(QSurface::OpenGLSurface);
    }

    void Update()
    {
        requestUpdate();
    }

    bool IsCreated() const
    {
        return created;
    }

protected:
    void exposeEvent(QExposeEvent* e)
    {
        Q_UNUSED(e);
        if (isExposed() && created == false)
        {
            created = true;
            createCallback();
        }

        if (isExposed() == false && created == true)
        {
            destroyCallback();
        }
    }

    bool event(QEvent* e)
    {
        if (e->type() == QEvent::UpdateRequest)
        {
            DVASSERT(created == true);
            frameCallback();
            return true;
        }

        return QWindow::event(e);
    }

private:
    DAVA::Function<void()> createCallback;
    DAVA::Function<void()> frameCallback;
    DAVA::Function<void()> destroyCallback;

    bool created = false;
};

class RenderWidgetOGL::ContextBinder
{
public:
    ContextBinder(QWindow* surface_)
        : surface(surface_)
    {
        DVASSERT(instance == nullptr);
        instance = this;
    }

    ~ContextBinder()
    {
        instance = nullptr;
    }

    virtual void AcquireContext() = 0;
    virtual void ReleaseContext() = 0;
    virtual void Present() = 0;

    static ContextBinder* instance;

protected:
    void EnsureContextCreated()
    {
        if (ctx == nullptr)
        {
            ctx = new QOpenGLContext();
            ctx->setFormat(surface->format());
            ctx->setScreen(surface->screen());
            ctx->create();
            DVASSERT(ctx->isValid() == true);
        }
    }

    QWindow* surface = nullptr;
    QOpenGLContext* ctx = nullptr;
};

RenderWidgetOGL::ContextBinder* RenderWidgetOGL::ContextBinder::instance = nullptr;

class RenderWidgetOGL::WorkThreadContextBinder : public RenderWidgetOGL::ContextBinder
{
public:
    WorkThreadContextBinder(QWindow* surface_)
        : ContextBinder(surface_)
    {
    }

    void AcquireContext() override
    {
        EnsureContextCreated();
        DVASSERT(ctx->thread() == QThread::currentThread());
        bool r = ctx->makeCurrent(surface);
        DVASSERT(r == true);
    }

    void ReleaseContext() override
    {
        if (ctx == nullptr)
        {
            return;
        }
        DVASSERT(ctx->thread() == QThread::currentThread());
        ctx->doneCurrent();
    }

    void Present() override
    {
        DVASSERT(ctx != nullptr);
        DVASSERT(ctx == QOpenGLContext::currentContext());
        bool r = ctx->makeCurrent(surface);
        DVASSERT(r == true);
        ctx->swapBuffers(surface);
    }
};

class RenderWidgetOGL::MainThreadContextBinder : public RenderWidgetOGL::ContextBinder
{
public:
    MainThreadContextBinder(QWindow* surface_)
        : ContextBinder(surface_)
    {
        DVASSERT(qApp->thread() == QThread::currentThread());
        EnsureContextCreated();
    }

    void AcquireContext() override
    {
        DVASSERT(qApp->thread() == QThread::currentThread());

        QSurface* prevSurface = nullptr;
        QOpenGLContext* prevContext = QOpenGLContext::currentContext();
        if (prevContext != nullptr)
        {
            prevSurface = prevContext->surface();
        }

        contextStack.emplace(prevSurface, prevContext);

        if (prevContext != ctx)
        {
            ctx->makeCurrent(surface);
        }
    }

    void ReleaseContext() override
    {
        DVASSERT(qApp->thread() == QThread::currentThread());
        DVASSERT(!contextStack.empty());
        QOpenGLContext* currentContext = QOpenGLContext::currentContext();

        ContextNode topNode = contextStack.top();
        contextStack.pop();

        if (topNode.context == currentContext)
        {
            return;
        }
        else if (currentContext != nullptr)
        {
            currentContext->doneCurrent();
        }

        if (topNode.context != nullptr && topNode.surface != nullptr)
        {
            topNode.context->makeCurrent(topNode.surface);
        }
    }

    void Present() override
    {
        DVASSERT(qApp->thread() == QThread::currentThread());
        DVASSERT(QOpenGLContext::currentContext() == ctx);
        ctx->makeCurrent(surface);
        ctx->swapBuffers(surface);
    }

private:
    struct ContextNode
    {
        ContextNode(QSurface* surface_ = nullptr, QOpenGLContext* context_ = nullptr)
            : surface(surface_)
            , context(context_)
        {
        }

        QSurface* surface = nullptr;
        QOpenGLContext* context = nullptr;
    };

    DAVA::Stack<ContextNode> contextStack;
};

void RenderWidgetOGL::AcquireContextStatic()
{
    DVASSERT(ContextBinder::instance != nullptr);
    ContextBinder::instance->AcquireContext();
}

void RenderWidgetOGL::ReleaseContextStatic()
{
    DVASSERT(ContextBinder::instance != nullptr);
    ContextBinder::instance->ReleaseContext();
}

void RenderWidgetOGL::PresentStatic()
{
    DVASSERT(ContextBinder::instance != nullptr);
    ContextBinder::instance->Present();
}

RenderWidgetOGL::RenderWidgetOGL(IWindowDelegate* windowDelegate, uint32 width, uint32 height, bool renderInThread_, QWidget* parent)
    : TBase(windowDelegate, width, height, parent)
    , renderInThread(renderInThread_)
{
    surface = new RenderSurface(MakeFunction(this, &RenderWidgetOGL::OnCreated),
                                MakeFunction(this, &RenderWidgetOGL::OnFrame),
                                MakeFunction(this, &RenderWidgetOGL::OnDestroyed));

    if (renderInThread == true)
    {
        binder = new WorkThreadContextBinder(surface);
    }
    else
    {
        binder = new MainThreadContextBinder(surface);
    }

    surface->installEventFilter(this);

    QWidget* container = QWidget::createWindowContainer(surface, this);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(container);
    container->setFocusProxy(this);
    setFocusPolicy(Qt::FocusPolicy::WheelFocus);

    setAutoFillBackground(false);
}

bool RenderWidgetOGL::IsInitialized() const
{
    return surface->IsCreated();
}

void RenderWidgetOGL::Update()
{
    surface->Update();
}

void RenderWidgetOGL::InitCustomRenderParams(rhi::InitParam& params)
{
    params.threadedRenderEnabled = false;
    params.threadedRenderFrameCount = 1;
    if (renderInThread == true)
    {
        params.threadedRenderEnabled = true;
        params.threadedRenderFrameCount = 2;
    }
    params.acquireContextFunc = &RenderWidgetOGL::AcquireContextStatic;
    params.releaseContextFunc = &RenderWidgetOGL::ReleaseContextStatic;
    params.presentFunc = &RenderWidgetOGL::PresentStatic;
    params.useBackBufferExtraSize = true;
}

void RenderWidgetOGL::AcquireContext()
{
    AcquireContextStatic();
}

void RenderWidgetOGL::ReleaseContext()
{
    ReleaseContextStatic();
}

bool RenderWidgetOGL::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == surface)
    {
        switch (e->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseMove:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Wheel:
            setFocus();
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::DragEnter:
        case QEvent::DragLeave:
        case QEvent::DragMove:
        case QEvent::Drop:
        case QEvent::NativeGesture:
            return event(e);
        default:
            break;
        }
    }

    return false;
}

bool RenderWidgetOGL::IsInFullScreen() const
{
    return surface->visibility() == QWindow::FullScreen;
}

void RenderWidgetOGL::OnCreated()
{
    TBase::OnCreated();
}

void RenderWidgetOGL::OnFrame()
{
    if (renderInThread == true)
    {
        OnFrameWorkThread();
    }
    else
    {
        OnFrameMainThread();
    }
}

void RenderWidgetOGL::OnFrameMainThread()
{
    if (firstFrame == true)
    {
        AcquireContext();
    }

    TBase::OnFrame();

    if (firstFrame == true)
    {
        ReleaseContext();
    }
    firstFrame = false;
}

void RenderWidgetOGL::OnFrameWorkThread()
{
    QOpenGLContext* ctx = nullptr;
    QOffscreenSurface* offSurface = nullptr;
    if (firstFrame == true)
    {
        ctx = new QOpenGLContext();
        ctx->setFormat(surface->format());
        ctx->setScreen(surface->screen());
        ctx->create();
        DVASSERT(ctx->isValid());

        offSurface = new QOffscreenSurface();
        offSurface->setFormat(surface->format());
        offSurface->setScreen(surface->screen());
        offSurface->create();
        DVASSERT(offSurface->isValid() == true);
        ctx->makeCurrent(offSurface);
    }

    TBase::OnFrame();
    if (firstFrame == true)
    {
        ctx->doneCurrent();
        offSurface->destroy();
        delete offSurface;
        delete ctx;
    };

    firstFrame = false;
}

void RenderWidgetOGL::OnDestroyed()
{
    TBase::OnDestroyed();
}

QWindow* RenderWidgetOGL::GetQWindow()
{
    return surface;
}

QPaintEngine* RenderWidgetOGL::paintEngine() const
{
    return nullptr;
}

} // namespace DAVA
#endif // __DAVAENGINE_QT__
