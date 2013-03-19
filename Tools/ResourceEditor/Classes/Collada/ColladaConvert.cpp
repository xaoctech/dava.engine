#include "ColladaConvert.h"
#include "ColladaDocument.h"

eColladaErrorCodes ConvertDaeToSce(const DAVA::String & pathToFile)
{
    FCollada::Initialize();
    
    DAVA::ColladaDocument colladaDocument;
    
    eColladaErrorCodes code = colladaDocument.Open(pathToFile.c_str());
    if (code != COLLADA_OK)
    {
        DAVA::Logger::Error("[ConvertDaeToSce] Failed to read %s with error %d", pathToFile.c_str(), (int32)code);
        return code;
    }
    
    /*
     int paramCount = CommandLineParser::Instance()->GetParamCount();
     for (int k = PARAM_SOURCE_FILEPATH + 1; k < paramCount; ++k)
     {
     colladaDocument.ExportAnimations(CommandLineParser::Instance()->GetParam(k).c_str());
     }
     */
    DAVA::String fileDirectory, filePath;
    DAVA::FileSystem::SplitPath(pathToFile, fileDirectory, filePath);
    filePath = DAVA::FileSystem::ReplaceExtension(filePath, ".sce");
    
    colladaDocument.SaveScene(fileDirectory, filePath);
    colladaDocument.Close();
    
    FCollada::Release();
    
    return COLLADA_OK;
}