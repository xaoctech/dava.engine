//
//  Crc32Helper.h
//  UIEditor
//
//  Created by Denis Bespalov on 3/18/13.
//
//

#ifndef __DAVAENGINE__CRC32__
#define __DAVAENGINE__CRC32__

#include "Base/BaseTypes.h"


namespace DAVA
{

class CRC32
{

public:
	
	static uint32 ForFile(const String & pathName);

};

}
#endif /* defined(__DAVAENGINE__CRC32__) */
