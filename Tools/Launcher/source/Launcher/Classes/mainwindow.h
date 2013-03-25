#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "installer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void AvailableSoftWareUpdated(const AvailableSoftWare&);
    void UpdateBtn();
    void OnLogAdded(const QString& log);

    void on_btnInstall_clicked();
    void on_btnRefresh_clicked();
    void on_btnRemove_clicked();
    void on_btnRun_clicked();

    void on_btnReinstall_clicked();

    void OnDownloadStarted();
    void OnDownloadProgress(int nPercent);
    void OnDownloadFinished();

    void on_btnCancel_clicked();

    void OnComboBoxValueChanged(const QString& value);

private:
    void FillTableSoft(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft);
    void FillTableDependencies(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft);
    void UpdateSelectedApp(QTableWidget* table);

private:
    Ui::MainWindow *ui;
    Installer* m_pInstaller;

    AvailableSoftWare m_SoftWare;

    eAppType m_SelectedAppType;
    QString m_SelectedApp;
    QString m_SelectedAppVersion;

    QTimer* m_pUpdateTimer;

    bool m_bBusy;
};

#endif // MAINWINDOW_H
