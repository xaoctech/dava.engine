#include "selfupdater.h"
#include <QNetworkRequest>
#include <QProcess>
#include "settings.h"
#include "logger.h"
#include "directorymanager.h"
#include "ziphelper.h"

#define NewLauncer "NewLauncer"
#define UnpacTempDir "NewLauncerTemp"
#define OldApp "Old"

SelfUpdater::SelfUpdater(QObject *parent) :
    QObject(parent)
{
    m_pNetworkManager = new QNetworkAccessManager(this);
    m_pReply = NULL;

    QString appDir = DirectoryManager::GetInstance()->GetAppDirectory();
#ifdef Q_OS_DARWIN
    QString oldDir = appDir + OldApp;
#elif defined Q_OS_WIN
    QString oldDir = qApp->applicationDirPath() + OldApp;
#endif
    while (!DirectoryManager::DeleteDir(oldDir)) {
        QThread().wait(100);
    }
}

SelfUpdater::~SelfUpdater()
{

}

void SelfUpdater::UpdatedConfigDownloaded(const AppsConfig& config) {
    m_UpdateLauncerConfig = config.m_Launcher;

    QString nNewVersion = Settings::GetVersion(config.m_Launcher.m_Version);
    QString nCurVersion = Settings::GetVersion(Settings::GetInstance()->GetLauncherVersion());
    if (nCurVersion != nNewVersion) {
        //download new version
        m_pReply = m_pNetworkManager->get(QNetworkRequest(config.m_Launcher.m_Url));
        connect(m_pReply, SIGNAL(finished()), this, SLOT(DownloadFinished()));
    }
}

void SelfUpdater::DownloadFinished()
{
    if(!m_pReply)
        return;

    QByteArray data = m_pReply->readAll();
    if (!data.size()) {
        Logger::GetInstance()->AddLog(tr("Error download new launcer version"));
        return;
    }

    QString strFilePath = DirectoryManager::GetInstance()->GetDownloadDir() + NewLauncer;
    QFile dataFile(strFilePath);
    dataFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    dataFile.write(data);
    dataFile.close();

    //delete old file
    QString strTempDir = DirectoryManager::GetInstance()->GetDownloadDir() + UnpacTempDir;
    DirectoryManager::DeleteDir(strTempDir);
    if (!QDir().mkdir(strTempDir))
    {
        Logger::GetInstance()->AddLog(tr("Error create temp directory"));
        return;
    }

    if (!zipHelper::unZipFile(strFilePath, strTempDir))
    {
        Logger::GetInstance()->AddLog(tr("Error unpack archive"));
        return;
    }

    //self update
    //rename cur app
#ifdef Q_OS_DARWIN
    QString appDir = DirectoryManager::GetInstance()->GetAppDirectory();
    if (!QDir().rename(appDir, appDir + OldApp))
    {
        Logger::GetInstance()->AddLog(tr("Error remove old version"));
        DirectoryManager::DeleteDir(strTempDir);
        return;
    }
#elif defined Q_OS_WIN
    QString appDir = qApp->applicationDirPath();
    QString oldDir = appDir + OldApp;
    DirectoryManager::DeleteDir(oldDir);
    if (!QDir().mkdir(oldDir))
    {
        Logger::GetInstance()->AddLog(tr("Error remove old version"));
        return;
    }

    QString appRunFile = qApp->applicationFilePath();
    appRunFile.remove(qApp->applicationDirPath());
    if (!QFile().rename(appDir + appRunFile, oldDir + appRunFile)) {
        Logger::GetInstance()->AddLog(tr("Error remove old version"));
        return;
    }
    QFile().rename(appDir + "/QtCore4.dll", oldDir + "/QtCore4.dll");
    QFile().rename(appDir + "/QtCored4.dll", oldDir + "/QtCored4.dll");
    QFile().rename(appDir + "/QtGui4.dll", oldDir + "/QtGui4.dll");
    QFile().rename(appDir + "/QtGuid4.dll", oldDir + "/QtGuid4.dll");
    QFile().rename(appDir + "/QtNetwork4.dll", oldDir + "/QtNetwork4.dll");
    QFile().rename(appDir + "/QtNetworkd4.dll", oldDir + "/QtNetworkd4.dll");
    QFile().rename(appDir + "/quazip1.dll", oldDir + "/quazip1.dll");
#endif

    QString baseDir = DirectoryManager::GetInstance()->GetBaseDirectory() + "/../";
    DirectoryManager::CopyAllFromDir(strTempDir, baseDir);
    DirectoryManager::DeleteDir(strTempDir);
    QFile().remove(strFilePath);

    Settings::GetInstance()->SetLauncherVersion(m_UpdateLauncerConfig.m_Version);
    qApp->quit();
    QProcess process;
    //process.startDetached(DirectoryManager::GetInstance()->GetRunPath());
    process.startDetached(qApp->applicationFilePath());
}
