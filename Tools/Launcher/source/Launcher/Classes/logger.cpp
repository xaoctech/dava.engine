#include "logger.h"

Logger* Logger::m_spLogger = NULL;

Logger* Logger::GetInstance() {
    if (!m_spLogger)
        m_spLogger = new Logger();
    return m_spLogger;
}

Logger::Logger(QObject *parent) :
    QObject(parent)
{
}

void Logger::AddLog(const QString& log){
    m_Logs.append(log);

    emit LogAdded(log);
}
