#include "Classes/Library/Private/DAEConverter.h"
#include "Classes/Collada/ColladaConvert.h"
#include "Classes/Collada/ImportParams.h"

#include <Base/Any.h>
#include <Asset/AssetManager.h>
#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>
#include <Engine/Engine.h>
#include <Scene3D/Prefab.h>

namespace DAEConverter
{
bool Convert(const DAVA::FilePath& daePath)
{
    DAVA::FileSystem* fileSystem = DAVA::GetEngineContext()->fileSystem;
    if (fileSystem->Exists(daePath) && daePath.IsEqualToExtension(".dae"))
    {
        std::unique_ptr<ImportParams> importParams = std::make_unique<ImportParams>();
        DAVA::FilePath etalonScenePath = daePath;
        etalonScenePath.ReplaceExtension(".prefab");
        if (fileSystem->Exists(etalonScenePath))
        {
            DAVA::RefPtr<DAVA::Scene> scene;
            scene.ConstructInplace();

            DAVA::Prefab::PathKey key(etalonScenePath);
            DAVA::Asset<DAVA::Prefab> prefabAsset = DAVA::GetEngineContext()->assetManager->GetAsset<DAVA::Prefab>(key, DAVA::AssetManager::SYNC);
            scene->ConstructFromPrefab(prefabAsset);

            AccumulateImportParams(scene, etalonScenePath, importParams.get());
        }

        eColladaErrorCodes code = ConvertDaeToPrefab(daePath, std::move(importParams));
        if (code == COLLADA_OK)
        {
            return true;
        }
        else if (code == COLLADA_ERROR_OF_ROOT_NODE)
        {
            DAVA::Logger::Error("Can't convert from DAE. Looks like one of materials has same name as root node.");
        }
        else
        {
            DAVA::Logger::Error("[DAE to Prefab] Can't convert from DAE.");
        }
    }
    else
    {
        DAVA::Logger::Error("[DAE to Prefab] Wrong pathname: %s.", daePath.GetStringValue().c_str());
    }

    return false;
}

bool ConvertAnimations(const DAVA::FilePath& daePath)
{
    DAVA::FileSystem* fileSystem = DAVA::GetEngineContext()->fileSystem;
    if (fileSystem->Exists(daePath) && daePath.IsEqualToExtension(".dae"))
    {
        eColladaErrorCodes code = ConvertDaeToAnimations(daePath);
        if (code == COLLADA_OK)
        {
            return true;
        }
        else
        {
            DAVA::Logger::Error("[DAE to animations] Can't convert from DAE.");
        }
    }
    else
    {
        DAVA::Logger::Error("[DAE to animations] Wrong pathname: %s", daePath.GetStringValue().c_str());
    }

    return false;
}
}
