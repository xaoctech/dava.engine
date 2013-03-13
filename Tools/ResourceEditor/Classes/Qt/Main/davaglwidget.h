#ifndef DAVAGLWIDGET_H
#define DAVAGLWIDGET_H

#include <QWidget>
#include <QTimer>

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

	void SetMaxFPS(int fps);
	int GetMaxFPS();
	int GetFPS();
    
	virtual QPaintEngine *paintEngine() const;
	
	virtual void paintEvent(QPaintEvent *);
	virtual void resizeEvent(QResizeEvent *);

	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    
#if defined (Q_WS_MAC)
    virtual void mouseMoveEvent(QMouseEvent *);
#endif //#if defined (Q_WS_MAC)
    
#if defined(Q_WS_WIN)
	virtual bool winEvent(MSG *message, long *result);
#endif //#if defined(Q_WS_WIN)

protected slots:
	void Render();

private:
	Ui::DavaGLWidget *ui;

	int maxFPS;
    int minFrameTimeMs;

	void Quit();
};

#endif // DAVAGLWIDGET_H
