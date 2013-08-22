#include "applicationmanager.h"
#include "filemanager.h"
#include "errormessanger.h"
#include "processhelper.h"
#include <QFile>
#include <QNetworkReply>
#include <QDebug>
#include <QMessageBox>

ApplicationManager::ApplicationManager(QObject *parent) :
    QObject(parent),
    localConfig(0),
    remoteConfig(0),
    currentDownload(0)
{
    networkManager = new QNetworkAccessManager();

    localConfigFilePath = FileManager::Instance()->GetDocumentsDirectory() + LOCAL_CONFIG_NAME;
    LoadLocalConfig(localConfigFilePath);
}

ApplicationManager::~ApplicationManager()
{
    SafeDelete(localConfig);
    SafeDelete(remoteConfig);

    SafeDelete(networkManager);
}

void ApplicationManager::LoadLocalConfig(const QString & configPath)
{
    QFile configFile(configPath);
    if(configFile.open(QFile::ReadWrite))
    {
        QByteArray data = configFile.readAll();
        localConfig = new ConfigParser(data);
    }
    else
    {
        ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_DOC_ACCESS);
        localConfig = new ConfigParser(QByteArray());
    }
}

bool ApplicationManager::ShouldShowNews()
{
    return remoteConfig && remoteConfig->GetNewsID() != localConfig->GetNewsID();
}

void ApplicationManager::NewsShowed()
{
    if(remoteConfig)
        localConfig->SetLastNewsID(remoteConfig->GetNewsID());
}

void ApplicationManager::CheckUpdates(QQueue<UpdateTask> & tasks)
{
    //check self-update
    if(remoteConfig && remoteConfig->GetLauncherVersion() != localConfig->GetLauncherVersion())
    {
        AppVersion version;
        version.id = remoteConfig->GetLauncherVersion();
        version.url = remoteConfig->GetLauncherURL();
        tasks.push_back(UpdateTask("", "", version, true));
    }

    //check applications update
    if(remoteConfig)
    {
        int branchCount = remoteConfig->GetBranchCount();
        for(int i = 0; i < branchCount; ++i)
        {
            Branch * branch = remoteConfig->GetBranch(i);
            if(!localConfig->GetBranch(branch->id))
                continue;

            int appCount = branch->GetAppCount();
            for(int j = 0; j < appCount; ++j)
            {
                Application * app = branch->GetApplication(j);
                if(!localConfig->GetApplication(branch->id, app->id))
                    continue;

                if(app->GetVerionsCount() == 1)
                {
                    AppVersion * appVersion = app->GetVersion(0);
                    Application * localApp = localConfig->GetApplication(branch->id, app->id);
                    if(localApp->GetVersion(0)->id != appVersion->id)
                        tasks.push_back(UpdateTask(branch->id, app->id, *appVersion));
                }
            }
        }
    }
}

void ApplicationManager::RefreshRemoteConfig()
{
    currentDownload = networkManager->get(QNetworkRequest(QUrl(localConfig->GetRemoteConfigURL())));
    connect(currentDownload, SIGNAL(finished()), this, SLOT(DownloadFinished()));
    connect(currentDownload, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(NetworkError(QNetworkReply::NetworkError)));
}

void ApplicationManager::OnAppInstalled(const QString & branchID, const QString & appID, const AppVersion & version)
{
    localConfig->InsertApplication(branchID, appID, version);
    localConfig->SaveToYamlFile(localConfigFilePath);
}

void ApplicationManager::NetworkError(QNetworkReply::NetworkError code)
{
    ErrorMessanger::Instance()->ShowErrorMessage(ErrorMessanger::ERROR_NETWORK, code, currentDownload->errorString());

    currentDownload->deleteLater();
    currentDownload = 0;
}

void ApplicationManager::DownloadFinished()
{
    if(currentDownload)
    {
        SafeDelete(remoteConfig);
        QByteArray data = currentDownload->readAll();
        if(data.size())
        {
            remoteConfig = new ConfigParser(data);
            QString webPageUrl = remoteConfig->GetWebpageURL();
            if(!webPageUrl.isEmpty())
                localConfig->SetWebpageURL(webPageUrl);
            localConfig->CopyStringsFromConfig(*remoteConfig);
            localConfig->SaveToYamlFile(localConfigFilePath);
        }
    }

    emit Refresh();
}

QString ApplicationManager::GetString(const QString & stringID)
{
    QString string = stringID;
    if(remoteConfig)
        string = remoteConfig->GetString(stringID);
    if(localConfig && string == stringID)
        string = localConfig->GetString(stringID);
    return string;
}

ConfigParser * ApplicationManager::GetRemoteConfig()
{
    return remoteConfig;
}

ConfigParser * ApplicationManager::GetLocalConfig()
{
    return localConfig;
}

void ApplicationManager::RunApplication(const QString & branchID, const QString & appID, const QString & versionID)
{
    AppVersion * version = localConfig->GetAppVersion(branchID, appID, versionID);
    if(version)
    {
        QString runPath = FileManager::Instance()->GetApplicationFolder(branchID, appID) + version->runPath;
        if(!ProcessHelper::IsProcessRuning(runPath))
            ProcessHelper::RunProcess(runPath);
    }
}

bool ApplicationManager::RemoveApplication(const QString & branchID, const QString & appID, const QString & versionID)
{
    AppVersion * version = localConfig->GetAppVersion(branchID, appID, versionID);
    if(version)
    {
        QString runPath = FileManager::Instance()->GetApplicationFolder(branchID, appID) + version->runPath;
        while(ProcessHelper::IsProcessRuning(runPath))
        {
            int result = ErrorMessanger::Instance()->ShowRetryDlg(true);
            if(result == QMessageBox::Cancel)
                return false;
        }

        QString appPath = FileManager::Instance()->GetApplicationFolder(branchID, appID);
        FileManager::Instance()->DeleteDirectory(appPath);
        localConfig->RemoveApplication(branchID, appID, versionID);
        localConfig->SaveToYamlFile(localConfigFilePath);

        return true;
    }
    return false;
}
