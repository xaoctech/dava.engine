#include "PVRConverter.h"
#include "Render/TextureDescriptor.h"

using namespace DAVA;

PVRConverter::PVRConverter()
{
	// pvr map
	pixelFormatToPVRFormat[DAVA::FORMAT_RGBA8888] = "OGLBGRA8888";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGBA4444] = "OGL4444";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGBA5551] = "OGL5551";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGB565] = "OGL565";
	pixelFormatToPVRFormat[DAVA::FORMAT_RGB888] = "OGL888";
	pixelFormatToPVRFormat[DAVA::FORMAT_PVR2] = "OGLPVRTC2";
	pixelFormatToPVRFormat[DAVA::FORMAT_PVR4] = "OGLPVRTC4";

	// dxt map
	// TODO:
	// ...

	dataFolderPath = FileSystem::Instance()->GetCurrentWorkingDirectory();
	std::replace(dataFolderPath.begin(), dataFolderPath.end(),'\\','/');
	String::size_type pos = dataFolderPath.find_first_of(":");
	if(String::npos != pos)
	{
		dataFolderPath = dataFolderPath.substr(pos + 1);
	}
}

PVRConverter::~PVRConverter()
{

}

String PVRConverter::ConvertPngToPvr(const String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	String outputName;
	String command = GetCommandLinePVR(fileToConvert, descriptor);

	if(!command.empty())
	{
		FileSystem::Instance()->Spawn(command);
		outputName = GetPVRToolOutput(fileToConvert);
	}

	return outputName;
}

String PVRConverter::ConvertPngToPvr(const String & fileToConvert, PixelFormat format, bool generateMimpaps)
{
    String filePath, pngFileName;
    FileSystem::SplitPath(fileToConvert, filePath, pngFileName);
    String pvrFileName = FileSystem::ReplaceExtension(pngFileName, ".pvr");
    
	String cwd = FileSystem::Instance()->GetCurrentWorkingDirectory();
	FileSystem::Instance()->SetCurrentWorkingDirectory(filePath);

	//GetPVRToolPath();
    
    String command = "";
#if defined (__DAVAENGINE_MACOS__)
    String converterPath = FileSystem::Instance()->SystemPathForFrameworkPath("~res:/PVRTexToolCL");
    
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
	String converterPath = FileSystem::Instance()->AbsoluteToRelativePath(filePath, dataFolderPath);
	converterPath += "/Data/PVRTexToolCL.exe";
    
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

String PVRConverter::GetCommandLinePVR(const DAVA::String & fileToConvert, const DAVA::TextureDescriptor &descriptor)
{
	String command;
	String format;

	command = GetPVRToolPath();
	format = pixelFormatToPVRFormat[descriptor.pvrCompression.format];

	if(command != "" && format != "")
	{
		String outputFile = GetPVRToolOutput(fileToConvert);

		// assemble command

		// input file
		command += Format(" -i \"%s\"", fileToConvert.c_str());

		// output format
		command += Format(" -f%s", format.c_str());

		// mipmaps
		if(descriptor.generateMipMaps)
		{
			command += " -m";
		}

		// TODO: base mipmap level
		// ...

		// flip vertical
		if(descriptor.pvrCompression.flipVertically)
		{
			command += " -yflip0";
		}

		// output file
		command += Format(" -o\"%s\"", outputFile.c_str());
	}

	return command;
}

String PVRConverter::GetPVRToolOutput(const DAVA::String &inputPVR)
{
	return FileSystem::ReplaceExtension(inputPVR, ".pvr");
}

String PVRConverter::GetPVRToolPath()
{
	String toolPath;

#if defined (__DAVAENGINE_MACOS__)
	toolPath = FileSystem::Instance()->SystemPathForFrameworkPath("~res:/PVRTexToolCL");
#elif defined (__DAVAENGINE_WIN32__)
	//toolPath = FileSystem::Instance()->AbsoluteToRelativePath(FileSystem::Instance()->GetCurrentWorkingDirectory(), dataFolderPath);
	toolPath = FileSystem::Instance()->GetCurrentWorkingDirectory();
	toolPath += "/Data/PVRTexToolCL.exe";
#endif

	if(!FileSystem::Instance()->IsFile(toolPath))
	{
		Logger::Error("PVRTexTool doesn't found in %s\n", toolPath.c_str());
		toolPath = "";
	}

	return toolPath;
}
