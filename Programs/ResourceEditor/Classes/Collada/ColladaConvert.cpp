#include "ColladaConvert.h"
#include "ColladaDocument.h"
#include "Classes/Collada/ColladaToSc2Importer/ColladaImporter.h"
#include "Classes/Collada/ImportParams.h"

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Scene3D/Prefab.h>

eColladaErrorCodes ConvertDaeToPrefab(const DAVA::FilePath& pathToFile, std::unique_ptr<DAEConverter::ImportParams>&& importParams)
{
    FCollada::Initialize();

    DAVA::ColladaDocument colladaDocument;

    eColladaErrorCodes code = colladaDocument.Open(pathToFile.GetAbsolutePathname().c_str());
    if (code != COLLADA_OK)
    {
        DAVA::Logger::Error("[ConvertDaeToPrefab] Failed to read %s with error %d", pathToFile.GetAbsolutePathname().c_str(), (int32)code);
        return code;
    }

    DAVA::FilePath prefabPath = DAVA::FilePath::CreateWithNewExtension(pathToFile, ".prefab");

    eColladaErrorCodes ret = colladaDocument.SavePrefab(prefabPath);
    colladaDocument.Close();

    FCollada::Release();

    if (ret == COLLADA_OK)
    {
        using namespace DAVA;

        ScopedPtr<Scene> scene(new Scene());

        Asset<Prefab> prefabAsset = GetEngineContext()->assetManager->LoadAsset<Prefab>(prefabPath);
        if (prefabAsset != nullptr)
        {
            scene->ConstructFromPrefab(prefabAsset);
        }
        else
        {
            DAVA::Logger::Error("[ConvertDaeToPrefab] Failed to read prefab %s for apply reimport params", pathToFile.GetAbsolutePathname().c_str());
        }

        DAEConverter::RestoreSceneParams(RefPtr<Scene>::ConstructWithRetain(scene), prefabPath, importParams.get());

        prefabAsset->ConstructFrom(scene);
        prefabAsset->Save(prefabAsset->GetFilepath());
    }

    return ret;
}

eColladaErrorCodes ConvertDaeToAnimations(const DAVA::FilePath& pathToFile)
{
    FCollada::Initialize();

    DAVA::ColladaDocument colladaDocument;

    eColladaErrorCodes code = colladaDocument.Open(pathToFile.GetAbsolutePathname().c_str(), true);
    if (code != COLLADA_OK)
    {
        DAVA::Logger::Error("[ConvertDaeToAnimations] Failed to read %s with error %d", pathToFile.GetAbsolutePathname().c_str(), (int32)code);
        return code;
    }

    DAVA::FilePath saveDirectory = pathToFile.GetDirectory();

    eColladaErrorCodes ret = colladaDocument.SaveAnimations(saveDirectory);
    colladaDocument.Close();

    FCollada::Release();

    return ret;
}
