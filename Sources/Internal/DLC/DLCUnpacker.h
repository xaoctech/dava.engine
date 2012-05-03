//
//  ZlibWrapper.h
//  WoTSniperMacOS
//
//  Created by Andrey Panasyuk on 3/21/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef WoTSniperMacOS_ZlibWrapper_h
#define WoTSniperMacOS_ZlibWrapper_h

#include "Base/BaseTypes.h"

#define CHUNK 16384

namespace DAVA
{

class File;
    
class DLCUnpacker
{
public:
    static void Unpack(const std::string& filePathSrc, const std::string& unpackPath, const std::string& fileForPaths = "");
    
    static int Inflate(File *source, File *dest);
};

}


#endif





