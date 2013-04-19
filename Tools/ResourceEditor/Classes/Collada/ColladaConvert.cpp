#include "ColladaConvert.h"
#include "ColladaDocument.h"

eColladaErrorCodes ConvertDaeToSce(const DAVA::FilePath & pathToFile)
{
    FCollada::Initialize();
    
    DAVA::ColladaDocument colladaDocument;
    
    eColladaErrorCodes code = colladaDocument.Open(pathToFile.GetAbsolutePathname().c_str());
    if (code != COLLADA_OK)
    {
        DAVA::Logger::Error("[ConvertDaeToSce] Failed to read %s with error %d", pathToFile.GetAbsolutePathname().c_str(), (int32)code);
        return code;
    }
    
    DAVA::FilePath path(pathToFile);
    path.ReplaceExtension(".sce");

    colladaDocument.SaveScene(path.GetDirectory().ResolvePathname(), path.GetFilename());
    colladaDocument.Close();
    
    FCollada::Release();
    
    return COLLADA_OK;
}