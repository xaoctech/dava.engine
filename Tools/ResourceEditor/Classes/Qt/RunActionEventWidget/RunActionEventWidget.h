#ifndef __RUNACTIONWIDGET_H__
#define __RUNACTIONWIDGET_H__


#include <QWidget>
#include <QScopedPointer>


namespace Ui {
    class RunActionEventWidget;
}


class RunActionEventWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit RunActionEventWidget( QWidget *parent = NULL );
    ~RunActionEventWidget();

private:
    QScopedPointer<Ui::RunActionEventWidget> ui;
};



#endif // __RUNACTIONWIDGET_H__
