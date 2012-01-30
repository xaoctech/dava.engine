#include "PVRConverter.h"

using namespace DAVA;

PVRConverter::PVRConverter()
{
	dataFolderPath = FileSystem::Instance()->GetCurrentWorkingDirectory();
	std::replace(dataFolderPath.begin(), dataFolderPath.end(),'\\','/');
	String::size_type pos = dataFolderPath.find_first_of(":");
	if(String::npos != pos)
	{
		dataFolderPath = dataFolderPath.substr(pos+1);
	}
}

PVRConverter::~PVRConverter()
{

}

void PVRConverter::ConvertPvrToPng(const String & fileToConvert)
{
    String filePath, pvrFileName;
    FileSystem::SplitPath(fileToConvert, filePath, pvrFileName);
    String pngFileName = pvrFileName+".png";
    
	String cwd = FileSystem::Instance()->GetCurrentWorkingDirectory();
	FileSystem::Instance()->SetCurrentWorkingDirectory(filePath);

// 	String converterPath = FileSystem::Instance()->SystemPathForFrameworkPath("~res:/PVRTexTool");
	String converterPath = FileSystem::Instance()->AbsoluteToRelativePath(filePath, dataFolderPath);
	converterPath += "/Data/PVRTexTool";
#if defined(__DAVAENGINE_WIN32__)
	converterPath = converterPath + ".exe";
//	converterPath = filePath + converterPath;
#endif //#if defined(__DAVAENGINE_WIN32__)

    String command = Format("\"\"%s\" -d -f8888 -i%s -o%s\"", converterPath.c_str(), pvrFileName.c_str(), pngFileName.c_str());
	Logger::Info(command.c_str());
	FileSystem::Instance()->Spawn(command);
 
	FileSystem::Instance()->SetCurrentWorkingDirectory(cwd);
}