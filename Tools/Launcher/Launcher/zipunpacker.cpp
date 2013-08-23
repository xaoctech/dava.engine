#include "zipunpacker.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <QDir>
#include <QFile>

#define PERM_OTHER_EXEC 1 << 0
#define PERM_OWNER_EXEC 1 << 3
#define PERM_GROUP_EXEC 1 << 6
#define PERM_DIRECTORY  1 << 8

ZipUnpacker::ZipUnpacker(QObject *parent) :
    QObject(parent)
{
    errorMap[0] = "UNZ_OK";
    errorMap[-1] = "UNZ_ERRNO";
    errorMap[-100] = "UNZ_END_OF_LIST_OF_FILE";
    errorMap[-102] = "UNZ_PARAMERROR";
    errorMap[-103] = "UNZ_BADZIPFILE";
    errorMap[-104] = "UNZ_INTERNALERROR";
    errorMap[-105] = "UNZ_CRCERROR";
}

ZipUnpacker::~ZipUnpacker()
{

}

const QString & ZipUnpacker::GetErrorString(int errorCode)
{
    if(errorMap.contains(errorCode))
        return errorMap[errorCode];
    else
        return errorMap[-1];
}

bool ZipUnpacker::UnZipFile(const QString& archiveFilePath, const QString& extDirPath)
{
    QuaZip zip(archiveFilePath);
    if (!zip.open(QuaZip::mdUnzip))
    {
        emit OnError(zip.getZipError());
        return false;
    }

    QDir().mkpath(extDirPath);

    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    QString name;

    int allcount = zip.getEntriesCount();
    int unzipCount = 0;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {

        if (!zip.getCurrentFileInfo(&info))
        {
            emit OnError(zip.getZipError());
            return false;
        }

        if (!file.open(QIODevice::ReadOnly))
        {
            emit OnError(zip.getZipError());
            return false;
        }

        name = QDir::toNativeSeparators(QString("%1/%2").arg(extDirPath).arg(file.getActualFileName()));

        if (file.getZipError() != UNZ_OK)
        {
            emit OnError(zip.getZipError());
            return false;
        }

        uint attr = info.externalAttr;
        attr = attr >> 16;  //leave only file permision info

        if (name.at(name.size() - 1) == '/' || name.at(name.size() - 1) == '\\')
        {
            //create directory
            QDir().mkdir(name);
        }
        else
        {
            QFile out;

            //unpack file
            out.setFileName(name);

            // this will fail if "name" contains subdirectories, but we don't mind that
            if (!out.open(QIODevice::WriteOnly))
            {
                qDebug("[ZipUnpacker::UnZipFile]: Error create file");
            }
            else
            {
                QFile::Permissions perm = out.permissions();
                perm |= QFile::ExeOwner;
                perm |= QFile::ExeGroup;
                perm |= QFile::ExeOther;

                if (!out.setPermissions(perm))
                  qDebug("[ZipUnpacker::UnZipFile] Error set file permision");

                out.write(file.readAll());
                out.close();
            }
        }

        if (file.getZipError() != UNZ_OK)
        {
            emit OnError(zip.getZipError());
            return false;
        }

        if (!file.atEnd()) {
            qWarning("[ZipUnpacker::UnZipFile]: read all but not EOF");
            return false;
        }

        file.close();

        if (file.getZipError() != UNZ_OK)
        {
            emit OnError(zip.getZipError());
            return false;
        }
        unzipCount++;

        emit OnProgress(unzipCount, allcount);
    }

    emit OnComplete();

    return true;
}
