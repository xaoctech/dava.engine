#include "PVRConverter.h"

void PVRConverter::ConvertPvrToPng(const String & fileToConvert)
{
    String filePath, pvrFileName;
    FileSystem::SplitPath(fileToConvert, filePath, pvrFileName);
    String pngFileName = pvrFileName+".png";
    
    String cwd = FileSystem::Instance()->GetCurrentWorkingDirectory();
    FileSystem::Instance()->SetCurrentWorkingDirectory(filePath);
    
    String converterPath = FileSystem::Instance()->SystemPathForFrameworkPath("~res:/PVRTexTool");
    FileSystem::Instance()->Spawn(Format("%s -d -f8888 -i%s -o%s", converterPath.c_str(), pvrFileName.c_str(), pngFileName.c_str()));
    
    FileSystem::Instance()->SetCurrentWorkingDirectory(cwd);
}