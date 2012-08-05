#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class GUIActionHandler;
class QtMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
   explicit QtMainWindow(QWidget *parent = 0);
   ~QtMainWindow();
    
private:
    void SetupMainMenu();
    void SetupProjectPath();
    void SetupDockWidgets();
    
private slots:

    void MenuFileWillShow();
    
private:
    GUIActionHandler *actionHandler;
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
