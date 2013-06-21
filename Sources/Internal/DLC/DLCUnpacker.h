#ifndef __DAVAENGINE_DLC_UNPACKER_H__
#define __DAVAENGINE_DLC_UNPACKER_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

#define CHUNK 16384

namespace DAVA
{

class File;
    
class DLCUnpacker
{
public:
    static void Unpack(const FilePath & filePathSrc, const FilePath & unpackPath, const FilePath & fileForPaths = FilePath());
    
    static int32 Inflate(File *source, File *dest);
};

}


#endif//__DAVAENGINE_DLC_UNPACKER_H__





