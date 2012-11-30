#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtPosSaver/QtPosSaver.h"

namespace Ui {
class MainWindow;
}

class LibraryModel;
class QtMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
   explicit QtMainWindow(QWidget *parent = 0);
   ~QtMainWindow();
    
    virtual bool eventFilter(QObject *, QEvent *);

    
private:
    void SetupMainMenu();
    
	void SetupToolBar();
	void DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename);

    void SetupProjectPath();
    void SetupDockWidgets();
    void SetupCustomColorsDock();
	void SetupVisibilityToolDock();
    
    void SetCustomColorsDockControlsEnabled(bool enabled);
        
private slots:

    void MenuFileWillShow();
	
	//reference
	void ApplyReferenceNodeSuffix();
        
private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;
    
    LibraryModel *libraryModel;

};


#endif // MAINWINDOW_H
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtPosSaver/QtPosSaver.h"

namespace Ui {
class MainWindow;
}

class QtMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
   explicit QtMainWindow(QWidget *parent = 0);
   ~QtMainWindow();
    
    virtual bool eventFilter(QObject *, QEvent *);
    
private:
    void SetupMainMenu();
    
	void SetupToolBar();
	void DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename);

    void SetupProjectPath();
    void SetupDockWidgets();
    void SetupCustomColorsDock();
    
    void SetCustomColorsDockControlsEnabled(bool enabled);
        
private slots:

    void MenuFileWillShow();
	
	//reference
	void ApplyReferenceNodeSuffix();
        
private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;
};


#endif // MAINWINDOW_H
