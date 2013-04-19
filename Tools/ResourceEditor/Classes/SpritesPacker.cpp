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

	// to prevent long relative path in $process(in FileSystem::RealPath(const String & _path)
	// '/' is skipped for win32!) and to avoid big code impact,  #ifdef(win32) was added  here
#if defined(__DAVAENGINE_WIN32__) 
	resourcePacker->excludeDirectory = FilePath("/")+inputDir + FilePath("../");
#else
	resourcePacker->excludeDirectory = inputDir + FilePath("../");
#endif
	resourcePacker->isLightmapsPacking = true;

	resourcePacker->PackResources();

	SafeDelete(resourcePacker);
}
