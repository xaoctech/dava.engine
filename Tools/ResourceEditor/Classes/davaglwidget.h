#ifndef DAVAGLWIDGET_H
#define DAVAGLWIDGET_H

#include <QWidget>

#include "Platform/Qt/QtLayer.h"

namespace Ui {
class DavaGLWidget;
}

class DavaGLWidget : public QWidget, public DAVA::QtLayerDelegate
{
    Q_OBJECT
    
public:
    explicit DavaGLWidget(QWidget *parent = 0);
    ~DavaGLWidget();
    
    virtual void Quit();

	virtual QPaintEngine *paintEngine() const;

    
protected:

	virtual void resizeEvent(QResizeEvent *);
    virtual void moveEvent(QMoveEvent *);

    virtual void paintEvent(QPaintEvent *);

	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	virtual void closeEvent(QCloseEvent *);
    
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    
#if defined (Q_WS_MAC)
    virtual void mouseMoveEvent(QMouseEvent *);
#endif //#if defined (Q_WS_MAC)
    
#if defined(Q_WS_WIN)
	virtual bool winEvent(MSG *message, long *result);
#endif //#if defined(Q_WS_WIN)

    
protected slots:
    
    void FpsTimerDone();
    void ReadyToQuit();
    
private:

	void InitFrameTimer();
	void DisableWidgetBlinking();

private:

    int frameTime;
	bool willClose;

private:
    Ui::DavaGLWidget *ui;
};

#endif // DAVAGLWIDGET_H
