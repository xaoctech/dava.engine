#include "SpritesPacker.h"
#include "ResourcePackerScreen.h"
#include "../TexturePacker/CommandLineParser.h"

SpritesPacker::~SpritesPacker()
{

}

void SpritesPacker::SetInputDir(const String & _inputDir)
{
	inputDir = _inputDir;
}

void SpritesPacker::SetOutputDir(const String & _outputDir)
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
	resourcePackerScreen->excludeDirectory = inputDir + "/../";
	resourcePackerScreen->isLightmapsPacking = true;

	resourcePackerScreen->PackResources();

	SafeRelease(resourcePackerScreen);
}
