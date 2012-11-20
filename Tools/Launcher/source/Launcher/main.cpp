#include <QtGui/QApplication>
#include "mainwindow.h"
#include "selfupdater.h"
#include "settings.h"
#include "configDownload.h"
#include "directorymanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DirectoryManager::GetInstance()->Init();
    Settings::GetInstance()->Init();

    MainWindow w;
    w.show();
    
    return a.exec();
}
