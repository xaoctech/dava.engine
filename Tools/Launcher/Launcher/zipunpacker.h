#ifndef ZIPHELPER_H
#define ZIPHELPER_H

#include <QObject>
#include <QString>
#include <QMap>
class ZipUnpacker: public QObject
{
    Q_OBJECT
public:
    explicit ZipUnpacker(QObject *parent = 0);
    ~ZipUnpacker();

    bool UnZipFile(const QString& archiveFilePath, const QString& extDirPath);

    const QString & GetErrorString(int errorCode);

signals:
    void OnProgress(int current, int all);
    void OnComplete();
    void OnError(int code);

private:
    QMap<int, QString> errorMap;
};

#endif // ZIPHELPER_H
