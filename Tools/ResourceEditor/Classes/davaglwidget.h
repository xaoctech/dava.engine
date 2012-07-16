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

	void timerEvent(QTimerEvent *);
	void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

	void showEvent(QShowEvent *);
	void hideEvent(QHideEvent *);


#if defined(Q_WS_WIN)
	virtual bool winEvent(MSG *message, long *result);
#endif //#if defined(Q_WS_WIN)

private:

    bool isFirstDraw;

    int counter;
    float divider;

private:
    Ui::DavaGLWidget *ui;
};

#endif // DAVAGLWIDGET_H
