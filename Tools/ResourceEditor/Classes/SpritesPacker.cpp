#include "SpritesPacker.h"
#include "ResourcePackerScreen.h"
#include "../TexturePacker/CommandLineParser.h"

SpritesPacker::~SpritesPacker()
{

}

void SpritesPacker::SetInputDir(const FilePath & _inputDir)
{
	inputDir = _inputDir;
}

void SpritesPacker::SetOutputDir(const FilePath & _outputDir)
{
	outputDir = _outputDir;
}

void SpritesPacker::Pack()
{
	FileSystem::Instance()->CreateDirectory(outputDir, true);

	ResourcePackerScreen * resourcePackerScreen = new ResourcePackerScreen();

	CommandLineParser::Instance()->ClearFlags(); //CommandLineParser is used in ResourcePackerScreen

	resourcePackerScreen->clearProcessDirectory = true;
	resourcePackerScreen->inputGfxDirectory = inputDir;
	resourcePackerScreen->outputGfxDirectory = outputDir;

	// to prevent long relative path in $process(in FileSystem::RealPath(const String & _path)
	// '/' is skipped for win32!) and to avoid big code impact,  #ifdef(win32) was added  here
#if defined(__DAVAENGINE_WIN32__) 
	resourcePackerScreen->excludeDirectory = "/"+FileSystem::Instance()->RealPath(inputDir + "/../");
#else
	resourcePackerScreen->excludeDirectory = FileSystem::Instance()->RealPath(inputDir + "/../");
#endif
	resourcePackerScreen->isLightmapsPacking = true;

	resourcePackerScreen->PackResources();

	SafeRelease(resourcePackerScreen);
}
