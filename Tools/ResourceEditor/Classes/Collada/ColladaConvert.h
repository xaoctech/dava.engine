#ifndef __COLLADA_CONVERT_H__
#define __COLLADA_CONVERT_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "ColladaErrorCodes.h"

eColladaErrorCodes ConvertDaeToSce(const DAVA::FilePath & pathToFile);

#endif // __COLLADA_CONVERT_H__