#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class GUIActionHandler;
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
	void resizeEvent(QResizeEvent *);

private:
    void SetupMainMenu();
    void SetupProjectPath();
    
private:
    GUIActionHandler *actionHandler;
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
