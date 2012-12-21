#ifndef LIBRARYWIDGET_H
#define LIBRARYWIDGET_H

#include <QWidget>

namespace Ui {
class LibraryWidget;
}

class LibraryWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit LibraryWidget(QWidget *parent = 0);
    ~LibraryWidget();
	    
private:
    Ui::LibraryWidget *ui;
};

#endif // LIBRARYWIDGET_H
