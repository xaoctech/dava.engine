#ifndef __SPRITES_PACKER_H__
#define __SPRITES_PACKER_H__

#include "DAVAEngine.h"
using namespace DAVA;

class SpritesPacker
{
public:
	virtual ~SpritesPacker();

	void SetInputDir(const String & inputDir);
	void SetOutputDir(const String & outputDir);
	void Pack();

protected:

	String inputDir;
	String outputDir;
};

#endif //__SPRITES_PACKER_H__