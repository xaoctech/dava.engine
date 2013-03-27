#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT
private:
    explicit Logger(QObject *parent = 0);

public:
    static Logger* GetInstance();
    
    void AddLog(const QString& log);

signals:
    void LogAdded(const QString& log);
    
public slots:
    
private:
    static Logger* m_spLogger;

    QList<QString> m_Logs;
};

#endif // LOGGER_H
