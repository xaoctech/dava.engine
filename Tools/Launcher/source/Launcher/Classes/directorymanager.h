#ifndef DIRECTORYMANAGER_H
#define DIRECTORYMANAGER_H

#include <QObject>

class DirectoryManager : public QObject
{
    Q_OBJECT
private:
    explicit DirectoryManager(QObject *parent = 0);
    
public:
    static DirectoryManager* GetInstance();
    static bool DeleteDir(const QString& path);
    static bool CopyAllFromDir(const QString& srcPath, const QString& destPath);
    static bool MoveFileToDir(const QString& srcFileName, const QString& destPath, const QString& newFileName = "");
    static QStringList GetDirectoryStructure(const QString& path);
    static bool IsFilePacked(const QString& fileName);

    void Init();

    QString GetBaseDirectory() const;
    QString GetAppDirectory() const;
    //QString GetRunPath() const {return m_runPath;}
    QString GetDownloadDir() const;
    QString GetStableDir() const;
    QString GetDevelopment() const;
    QString GetDependencies() const;
    QString GetConfigDir() const;


signals:
    
public slots:
    
private:
    void InitBaseDir();

    static DirectoryManager* m_spInstance;

    QString m_appDir;
    QString m_runPath;
    QString m_configDir;
};

#endif // DIRECTORYMANAGER_H
