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

	void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

	void showEvent(QShowEvent *);
	void hideEvent(QHideEvent *);

	void closeEvent(QCloseEvent *);

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
