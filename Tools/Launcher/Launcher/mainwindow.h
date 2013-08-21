#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "applicationmanager.h"
#include "buttonswidget.h"
#include <QMainWindow>
#include <QtGui>
#include <QSet>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void OnRefreshClicked();
    void OnListItemClicked(QModelIndex);
    void RefreshApps();
    void OnURLClicked();

    void OnRun(int rowNumber);
    void OnInstall(int rowNumber);
    void OnRemove(int rowNumber);

    void OnCellClicked(const QPoint & pos);
    void OnCellDoubleClicked(QModelIndex index);

private:
    void ShowWebpage();
    void ShowTable(const QString & branchID);
    void ShowUpdateDialog(QQueue<UpdateTask> & tasks);

    void UpdateURLValue();

    void RefreshBranchesList();
    void UpdateButtonsState(int rowNumber, ButtonsWidget::ButtonsState state);

    void GetTableApplicationIDs(int rowNumber, QString & appID, QString & installedVersionID, QString & avalibleVersionID);

    QListWidgetItem * CreateListItem(const QString & stringID);
    QWidget * CreateAppNameTableItem(const QString & stringID);
    QWidget * CreateAppInstalledTableItem(const QString & stringID);
    QWidget * CreateAppAvalibleTableItem(Application * app);

    Ui::MainWindow *ui;
    ApplicationManager * appManager;

    QModelIndex selectedListItem;
    QString selectedBranchID;

    QFont listFont;
    QFont tableFont;
};

#endif // MAINWINDOW_H
