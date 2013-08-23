#include "mainwindow.h"
#include "filemanager.h"
#include <errormessanger.h>
#include <QApplication>

void LogMsgHandler(QtMsgType type, const char * msg)
{
    ErrorMessanger::Instance()->LogMessage(type, msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ErrorMessanger::Instance();
    qInstallMsgHandler(LogMsgHandler);

    MainWindow w;
    w.show();
    w.setWindowState(Qt::WindowActive);
    
/////////TEMP!!! Delete previous Launcher version
/////////Remove in next version
    QString appDir = FileManager::Instance()->GetLauncherDirectory();
    FileManager::Instance()->DeleteDirectory(appDir + "../Dependencies");
    FileManager::Instance()->DeleteDirectory(appDir + "../Development");
    FileManager::Instance()->DeleteDirectory(appDir + "../Downloads");
    FileManager::Instance()->DeleteDirectory(appDir + "../Launcherold");
    FileManager::Instance()->DeleteDirectory(appDir + "Launcher.appold");
    FileManager::Instance()->DeleteDirectory(appDir + "../QA");
    FileManager::Instance()->DeleteDirectory(appDir + "../Stable");
    FileManager::Instance()->DeleteDirectory(appDir + "../ToMaster");
    QFile file(appDir + "settings.yaml");
    file.remove();
//////////////////////////////////////////////////

    return a.exec();
}
