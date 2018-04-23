#include <Asset/Asset.h>
#include <Asset/AssetManager.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Render/Material/Material.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialReflection.h>

#include <UnitTests/UnitTests.h>

#include <iostream>

DAVA_TESTCLASS (NMaterialReflectionTest)
{
    DAVA_TEST (ReflectionTest)
    {
        using namespace DAVA;
        FilePath materialPath = "~res:/3d/Materials/material.mat";

        Material::PathKey key(materialPath);
        Asset<Material> materialAsset = GetEngineContext()->assetManager->GetAsset<Material>(key, AssetManager::SYNC);
        NMaterial* material = materialAsset->GetMaterial();
        Reflection r = Reflection::Create(material);
        r.Dump(std::cout);
    }
};
