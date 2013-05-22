#include "SpritesPacker.h"
#include "TexturePacker/ResourcePacker2D.h"
#include "TexturePacker/CommandLineParser.h"

SpritesPacker::~SpritesPacker()
{

}

void SpritesPacker::SetInputDir(const FilePath & _inputDir)
{
    DVASSERT(_inputDir.IsDirectoryPathname());
	inputDir = _inputDir;
}

void SpritesPacker::SetOutputDir(const FilePath & _outputDir)
{
    DVASSERT(_outputDir.IsDirectoryPathname());
	outputDir = _outputDir;
}

void SpritesPacker::Pack()
{
	FileSystem::Instance()->CreateDirectory(outputDir, true);

	ResourcePacker2D * resourcePacker = new ResourcePacker2D();

	CommandLineParser::Instance()->ClearFlags(); //CommandLineParser is used in ResourcePackerScreen

	resourcePacker->clearProcessDirectory = true;
	resourcePacker->inputGfxDirectory = inputDir;
	resourcePacker->outputGfxDirectory = outputDir;

	resourcePacker->excludeDirectory = inputDir + "../";

	resourcePacker->isLightmapsPacking = true;

	resourcePacker->PackResources();

	SafeDelete(resourcePacker);
}
