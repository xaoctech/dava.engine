#pragma once

#include <Logger/Logger.h>
#include <FileSystem/FilePath.h>

class ServerLogger : public DAVA::LoggerOutput
{
public:
    ServerLogger(const DAVA::FilePath& logFilePath);
    ~ServerLogger();
    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;

private:
    DAVA::FilePath logFile;
};
