#include "ziphelper.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <QDir>
#include <QFile>

#define PERM_OTHER_EXEC 1 << 0
#define PERM_OWNER_EXEC 1 << 3
#define PERM_GROUP_EXEC 1 << 6
#define PERM_DIRECTORY  1 << 8

bool zipHelper::unZipFile(const QString& archiveFilePath, const QString& extDirPath)
{
    QuaZip zip(archiveFilePath);
    if (!zip.open(QuaZip::mdUnzip)) {
        qWarning("testRead(): zip.open(): %d", zip.getZipError());
        return false;
    }

    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    QString name;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {

        if (!zip.getCurrentFileInfo(&info)) {
            qWarning("testRead(): getCurrentFileInfo(): %d\n", zip.getZipError());
            return false;
        }

        /*if (!singleFileName.isEmpty())
            if (!info.name.contains(singleFileName))
                continue;*/

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("testRead(): file.open(): %d", file.getZipError());
            return false;
        }

        name = QString("%1/%2").arg(extDirPath).arg(file.getActualFileName());
        //qDebug(name.toAscii());

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }

        uint attr = info.externalAttr;
        attr = attr >> 16;  //leave only file permision info

        if (name.at(name.size() - 1) == '/' || name.at(name.size() - 1) == '\\')
        //if (attr & PERM_DIRECTORY)
        {
            //create directory
            QDir().mkdir(name);
        }
        else
        {
            char c;
            QFile out;

            //unpack file
            out.setFileName(name);

            // this will fail if "name" contains subdirectories, but we don't mind that
            if (!out.open(QIODevice::WriteOnly)) {
                qDebug("Error create file");
                //continue;
            } else {
                QFile::Permissions perm = out.permissions();
                if (attr & PERM_OWNER_EXEC)
                    perm |= QFile::ExeOwner;
                if (attr & PERM_GROUP_EXEC)
                    perm |= QFile::ExeGroup;
                if (attr & PERM_OTHER_EXEC)
                    perm |= QFile::ExeOther;

                if (!out.setPermissions(perm))
                  qDebug("Error set file permision");

                // Slow like hell (on GNU/Linux at least), but it is not my fault.
                // Not ZIP/UNZIP package's fault either.
                // The slowest thing here is out.putChar(c).
                while (file.getChar(&c)) {
                    out.putChar(c);
                }

                out.close();
            }
        }

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.getFileName(): %d", file.getZipError());
            return false;
        }

        if (!file.atEnd()) {
            qWarning("testRead(): read all but not EOF");
            return false;
        }

        file.close();

        if (file.getZipError() != UNZ_OK) {
            qWarning("testRead(): file.close(): %d", file.getZipError());
            return false;
        }
    }

    return true;
}
