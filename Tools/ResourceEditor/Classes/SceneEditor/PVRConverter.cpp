#include "PVRConverter.h"

using namespace DAVA;

PVRConverter::PVRConverter()
{
	dataFolderPath = String("");
}

PVRConverter::~PVRConverter()
{

}

String PVRConverter::GetDAVAPathname(const String &pathname)
{
	String workingPathname = pathname;
	std::replace(workingPathname.begin(), workingPathname.end(),'\\','/');
	String::size_type pos = workingPathname.find_first_of(":");
	if(String::npos != pos)
	{
		workingPathname = workingPathname.substr(pos+1);
	}
	
	return workingPathname;
}



String PVRConverter::ConvertPngToPvr(const String & fileToConvert, PixelFormat format, bool generateMimpaps)
{
    String filePath, pngFileName;
    FileSystem::SplitPath(GetDAVAPathname(fileToConvert), filePath, pngFileName);
    String pvrFileName = FileSystem::ReplaceExtension(pngFileName, ".pvr");
    
	String cwd = FileSystem::Instance()->GetCurrentWorkingDirectory();
	FileSystem::Instance()->SetCurrentWorkingDirectory(filePath);
    
    String command = "";
#if defined (__DAVAENGINE_MACOS__)
    String converterPath = FileSystem::Instance()->SystemPathForFrameworkPath(pvrTexToolPathname + "PVRTexToolCL");
    
    switch (format)
    {
        case FORMAT_PVR4:
            command = Format("%s -fOGLPVRTC4 -i%s -yflip0", converterPath.c_str(), pngFileName.c_str());
            break;

        case FORMAT_PVR2:
            command = Format("%s -fOGLPVRTC2 -i%s -yflip0", converterPath.c_str(), pngFileName.c_str());
            break;
            
        default:
            break;
    }
    
    if(generateMimpaps && command.length())
    {
        command += " -m";
    }
    
#elif defined (__DAVAENGINE_WIN32__)

	String converterPath = FileSystem::Instance()->AbsoluteToRelativePath(filePath, dataFolderPath + pvrTexToolPathname + "PVRTexToolCL.exe");
    switch (format)
    {
        case FORMAT_PVR4:
            if(generateMimpaps)
            {
                command = Format("\"\"%s\" -fOGLPVRTC4 -i%s -m -yflip0\"", converterPath.c_str(), pngFileName.c_str());
            }
            else
            {
                command = Format("\"\"%s\" -fOGLPVRTC4 -i%s -yflip0\"", converterPath.c_str(), pngFileName.c_str());
            }
            break;
            
        case FORMAT_PVR2:
            if(generateMimpaps)
            {
                command = Format("\"\"%s\" -fOGLPVRTC2 -i%s -m -yflip0\"", converterPath.c_str(), pngFileName.c_str());
            }
            else
            {
                command = Format("\"\"%s\" -fOGLPVRTC2 -i%s -yflip0\"", converterPath.c_str(), pngFileName.c_str());
            }
            break;
            
        default:
            break;
    }
#endif    
    
	Logger::Info(command.c_str());
	FileSystem::Instance()->Spawn(command);
    
	FileSystem::Instance()->SetCurrentWorkingDirectory(cwd);
    
    String retPvrName = filePath + pvrFileName;
    return retPvrName;
}

void PVRConverter::SetPVRTexTool(const DAVA::String dataPathname, const DAVA::String &textToolPathname)
{
	dataFolderPath = GetDAVAPathname(dataPathname);
	pvrTexToolPathname = GetDAVAPathname(textToolPathname);
}
