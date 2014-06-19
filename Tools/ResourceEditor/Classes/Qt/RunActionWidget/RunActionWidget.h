#ifndef __RUNACTIONWIDGET_H__
#define __RUNACTIONWIDGET_H__


#include <QWidget>
#include <QScopedPointer>

#include "Interfaces/ISceneWatcher.h"


namespace Ui {
    class RunActionWidget;
}


class RunActionWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit RunActionWidget( QWidget *parent = NULL );
    ~RunActionWidget();

private:
    QScopedPointer<Ui::RunActionWidget> ui;
};


#endif // __RUNACTIONWIDGET_H__
