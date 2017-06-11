#include "QtHelpers/HelperFunctions.h"

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDir>
#include <QDirIterator>

namespace QtHelpers
{
void ShowInOSFileManager(const QString& path)
{
#if defined(Q_OS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \"" + path + "\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined(Q_OS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(path);
    QProcess::startDetached("explorer", args);
#endif //
}

//realisation for windows, only calls given function
#ifdef Q_OS_WIN
void InvokeInAutoreleasePool(std::function<void()> function)
{
    function();
}
#endif

void CopyRecursively(const QString& fromDir, const QString& toDir)
{
    QDir().mkpath(toDir);
 
    QString toDirWithSlash = toDir;
    if (!toDirWithSlash.endsWith('/'))
    {
        toDirWithSlash.append('/');
    }

    QDirIterator it(fromDir, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext())
    {
        it.next();
        
        QFileInfo fileInfo = it.fileInfo();
        QString dest = toDirWithSlash + fileInfo.fileName();
        
        if (fileInfo.isDir())
        {
            CopyRecursively(fileInfo.absoluteFilePath(), dest);
        }
        else
        {
            QFile::copy(fileInfo.absoluteFilePath(), dest);
        }
    }
}

} // namespace QtHelpers
