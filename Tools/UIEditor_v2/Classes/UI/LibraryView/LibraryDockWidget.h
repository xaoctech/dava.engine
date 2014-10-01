//
//  LibraryDockWidget.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/27/14.
//
//

#ifndef __UIEditor__LibraryDockWidget__
#define __UIEditor__LibraryDockWidget__

#include <QWidget>
#include <QDockWidget>

namespace Ui {
    class LibraryDockWidget;
}

class LibraryDockWidget: public QDockWidget
{
    Q_OBJECT
public:
    explicit LibraryDockWidget(QWidget *parent = NULL);
    virtual ~LibraryDockWidget();

private:
    Ui::LibraryDockWidget *ui;
};

#endif /* defined(__UIEditor__LibraryDockWidget__) */
