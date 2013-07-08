#ifndef __SPRITES_PACKER_H__
#define __SPRITES_PACKER_H__

#include "DAVAEngine.h"
using namespace DAVA;

class SpritesPacker
{
public:
	virtual ~SpritesPacker();

	void SetInputDir(const FilePath & inputDir);
	void SetOutputDir(const FilePath & outputDir);
	void Pack();

protected:

	FilePath inputDir;
	FilePath outputDir;
};

#endif //__SPRITES_PACKER_H__