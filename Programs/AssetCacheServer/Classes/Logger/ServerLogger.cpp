#pragma once

#include "Logger/ServerLogger.h"

#include <Time/DateTime.h>
#include <FileSystem/FileSystem.h>
#include <Utils/StringFormat.h>

namespace ServerLoggerDetails
{
const DAVA::uint32 FILE_MAX_SIZE = 30 * 1024 * 1024;
const DAVA::uint32 FILES_COUNT = 10;
const size_t LINE_MAX_LENGTH = 128;
}

ServerLogger::ServerLogger(const DAVA::FilePath& logFilePath)
{
    logFile = logFilePath;
    DAVA::Logger::AddCustomOutput(this);
}

ServerLogger::~ServerLogger()
{
    DAVA::Logger::RemoveCustomOutput(this);
}

void ServerLogger::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    using namespace DAVA;
    using namespace ServerLoggerDetails;

    FileSystem* fs = FileSystem::Instance();
    if (fs)
    {
        uint64 fileSize = 0;
        if (fs->GetFileSize(logFile, fileSize) && fileSize > FILE_MAX_SIZE)
        {
            for (int i = FILES_COUNT; i > 0; --i)
            {
                FilePath newName = logFile + Format(".%i", i);
                FilePath currentName = logFile;
                if (i > 1)
                    currentName += Format(".%i", i - 1);
                currentName += ".txt";

                if (fs->Exists(currentName))
                    fs->MoveFile(currentName, newName, true);
            }
        }
        ScopedPtr<File> file(File::Create(logFile, File::APPEND | File::WRITE));
        if (file)
        {
            Array<char8, LINE_MAX_LENGTH> prefix;
            DateTime dt = DateTime::Now();
            Snprintf(prefix.data(), prefix.size(), "%2.i/%2.i/%2.i %2.i:%2.i:%2.i [%s] ", dt.GetDay(), dt.GetMonth(), dt.GetYear(), dt.GetHour(), dt.GetMinute(), dt.GetSecond(), Logger::GetLogLevelString(ll));
            file->Write(prefix.data(), static_cast<uint32>(strlen(prefix.data())));
            file->Write(text, static_cast<uint32>(strlen(text)));
        }
    }
}
