#include "Utils.h"

DAVA::String Utils::GetContentOfFile(const DAVA::FilePath& path)
{
    DAVA::RefPtr<DAVA::File> file(DAVA::File::Create(path, DAVA::File::OPEN | DAVA::File::READ));
    if (!file.Valid())
    {
        DAVA::Logger::Error("File \"%s\" not found.", path.GetAbsolutePathname().c_str());
        return "";
    }

    DAVA::uint32 buffSize = file->GetSize();
    if (buffSize == 0)
    {
        DAVA::Logger::Error("File \"%s\" is empty.", path.GetAbsolutePathname().c_str());
        return "";
    }

    std::unique_ptr<char[]> data(new char[buffSize + 1]);
    DAVA::uint32 buffSizeRead = file->Read(data.get(), buffSize);
    data[buffSizeRead] = 0;

    if (buffSizeRead != buffSize)
    {
        DAVA::Logger::Error("Read amount of data from file \"%s\" has unexpected size. Expect %d, actual %d.", path.GetAbsolutePathname().c_str(), buffSize, buffSizeRead);
        return "";
    }

    return DAVA::String(data.get());
}
