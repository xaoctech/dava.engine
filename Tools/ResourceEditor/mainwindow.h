#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Classes/SceneEditor/EditorSettings.h"


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
    
private slots:
    void MenuFileWillShow();
    void ResentSceneTriggered(QAction *resentScene);

    
private:
    void SetupMainMenu();
    void SetupProjectPath();
    
private:
    GUIActionHandler *actionHandler;
    
    QAction *resentSceneActions[EditorSettings::RESENT_FILES_COUNT];

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
