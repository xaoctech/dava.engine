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
	
	void AddControl(const QString& name);
	void RemoveControl(const QString& name);
	void UpdateControl(const QString& oldName, const QString& name);
	
private:
    Ui::LibraryWidget *ui;
};

#endif // LIBRARYWIDGET_H
