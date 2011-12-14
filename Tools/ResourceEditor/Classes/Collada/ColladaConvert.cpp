#include "ColladaConvert.h"
#include "ColladaDocument.h"


void ConvertDaeToSce(const DAVA::String & pathToFile)
{
    FCollada::Initialize();
    
    DAVA::ColladaDocument colladaDocument;
    if (!colladaDocument.Open(pathToFile.c_str()))
    {
        printf("*** ERROR: Failed to read %s\n", pathToFile.c_str());
        return;
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
}