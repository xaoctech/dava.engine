#ifndef DAVAGLWIDGET_H
#define DAVAGLWIDGET_H

#include <QWidget>

namespace Ui {
class DavaGLWidget;
}

class DavaGLWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit DavaGLWidget(QWidget *parent = 0);
    ~DavaGLWidget();
    
protected:

	virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);

	virtual void showEvent(QShowEvent *);
	virtual void hideEvent(QHideEvent *);

	virtual void closeEvent(QCloseEvent *);

    virtual void moveEvent(QMoveEvent *);

    
#if defined(Q_WS_WIN)
	virtual bool winEvent(MSG *message, long *result);
#endif //#if defined(Q_WS_WIN)

    
protected slots:
    
    void FpsTimerDone();
    
private:

    QTimer *fpsTimer;
    int frameTime;
	bool willClose;

private:
    Ui::DavaGLWidget *ui;
};

#endif // DAVAGLWIDGET_H
