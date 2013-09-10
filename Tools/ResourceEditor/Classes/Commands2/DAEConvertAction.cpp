/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "DAEConvertAction.h"
#include "Collada/ColladaConvert.h"

#include "Classes/SceneEditor/SceneValidator.h"
#include "Classes/Qt/DockLODEditor/EditorLODData.h"

using namespace DAVA;

DAEConvertAction::DAEConvertAction(const DAVA::FilePath &path)
	: CommandAction(CMDID_DAE_CONVERT, "DAE to SC2 Convert")
	, daePath(path)
{ }

void DAEConvertAction::Redo()
{
	if(daePath.Exists() && daePath.IsEqualToExtension(".dae"))
	{
		eColladaErrorCodes code = ConvertDaeToSce(daePath.GetAbsolutePathname());
		if(code == COLLADA_OK)
		{
            ConvertFromSceToSc2();
        }
		else if(code == COLLADA_ERROR_OF_ROOT_NODE)
		{
			Logger::Error("Can't convert from DAE. Looks like one of materials has same name as root node.");
		}
		else
		{
			Logger::Error("Can't convert from DAE.");
		}
	}
}

void DAEConvertAction::ConvertFromSceToSc2() const
{
    Scene *scene = CreateSceneFromSce();

    // Export to *.sc2
    FilePath sc2Path = FilePath::CreateWithNewExtension(daePath, ".sc2");
    SaveScene(sc2Path, scene);
    scene->Release();
}


DAVA::Scene * DAEConvertAction::CreateSceneFromSce() const
{
    FilePath scePath = FilePath::CreateWithNewExtension(daePath, ".sce");
    
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(scePath);
    scene->AddNode(rootNode);
    scene->BakeTransforms();
    
    return scene;
}

void DAEConvertAction::SaveScene(const DAVA::FilePath &scenePathname, DAVA::Scene *scene) const
{
    SceneFileV2 * file = new SceneFileV2();
    file->EnableDebugLog(false);
    file->SaveScene(scenePathname, scene);
    
    SafeRelease(file);
}


DAEConvertWithSettingsAction::DAEConvertWithSettingsAction(const DAVA::FilePath &path)
    : DAEConvertAction(path)
{
    
}

void DAEConvertWithSettingsAction::ConvertFromSceToSc2() const
{
    FilePath sc2Path = FilePath::CreateWithNewExtension(daePath, ".sc2");
    if(sc2Path.Exists())
    {
        Scene *scene = CreateSceneFromSce();
        
        FilePath newSc2Path = sc2Path;
        newSc2Path.ReplaceBasename(sc2Path.GetBasename() + "_new");
        SaveScene(newSc2Path, scene);
        SafeRelease(scene);
        
        TryToMergeScenes(sc2Path, newSc2Path);
        
        FileSystem::Instance()->DeleteFile(newSc2Path);
    }
    else
    {
        DAEConvertAction::ConvertFromSceToSc2();
    }
}


void DAEConvertWithSettingsAction::TryToMergeScenes(const DAVA::FilePath &originalPath, const DAVA::FilePath &newPath) const
{
    Scene * oldScene = CreateSceneFromSc2(originalPath);
    Scene * newScene = CreateSceneFromSc2(newPath);
    
    CopyMaterialsSettings(oldScene, newScene);
    CopyLODSettings(oldScene, newScene);
    
    SaveScene(originalPath, newScene);
    
    oldScene->Release();
    newScene->Release();
}

DAVA::Scene * DAEConvertWithSettingsAction::CreateSceneFromSc2(const DAVA::FilePath &scenePathname) const
{
    Scene * scene = new Scene();
    
    Entity * rootNode = scene->GetRootNode(scenePathname);
	if(rootNode)
	{
		Vector<Entity*> tmpEntities;
		uint32 entitiesCount = (uint32)rootNode->GetChildrenCount();
        
		// optimize scene
        SceneFileV2 *sceneFile = new SceneFileV2();
		sceneFile->OptimizeScene(rootNode);
		sceneFile->Release();
        
		// remember all child pointers, but don't add them to scene in this cycle
		// because when entity is adding it is automatically removing from its old hierarchy
		tmpEntities.reserve(entitiesCount);
		for (uint32 i = 0; i < entitiesCount; ++i)
		{
			tmpEntities.push_back(rootNode->GetChild(i));
		}
        
		// now we can safely add entities into our hierarchy
		for (uint32 i = 0; i < (uint32) tmpEntities.size(); ++i)
		{
			scene->AddNode(tmpEntities[i]);
		}

        Set<String> errorsLog;
        SceneValidator::Instance()->ValidateScene(scene, errorsLog);
	}
    
    return scene;
}


void DAEConvertWithSettingsAction::CopyMaterialsSettings(DAVA::Scene * srcScene, DAVA::Scene * dstScene) const
{
    Vector<Material *> srcMaterials;
    srcScene->GetDataNodes(srcMaterials);
    
    Vector<Material *> dstMaterials;
    dstScene->GetDataNodes(dstMaterials);
    
    if (srcMaterials.size() == dstMaterials.size())
    {
        uint32 count = srcMaterials.size();
        for (uint32 i = 0; i < count; ++i)
        {
            CopyMaterial(srcMaterials[i], dstMaterials[i]);
        }
    }
}

void DAEConvertWithSettingsAction::CopyMaterial(DAVA::Material *src, DAVA::Material *dst) const
{
    if(src->GetName() != dst->GetName())
        return;
    
    dst->SetType((Material::eType)src->type);
    dst->SetViewOption(src->GetViewOption());
    
    dst->reflective = src->reflective;
    dst->reflectivity =	src->reflectivity;
    
    dst->transparent = src->transparent;
    dst->transparency =	src->transparency;
    dst->indexOfRefraction = src->indexOfRefraction;
    
    for(uint32 i = 0; i < Material::TEXTURE_COUNT; ++i)
    {
        dst->SetTexture((Material::eTextureLevel)i, src->GetTextureName((Material::eTextureLevel)i));
    }

    dst->SetBlendSrc(src->GetBlendSrc());
    dst->SetBlendDest(src->GetBlendDest());
    
    dst->SetOpaque(src->GetOpaque());
    dst->SetTwoSided(src->GetTwoSided());
    
    dst->SetSetupLightmap(src->GetSetupLightmap());
    
    dst->SetShininess(src->GetShininess());
    
    dst->SetAmbientColor(src->GetAmbientColor());
    dst->SetDiffuseColor(src->GetDiffuseColor());
    dst->SetSpecularColor(src->GetSpecularColor());
    dst->SetEmissiveColor(src->GetEmissiveColor());

    dst->SetFog(src->IsFogEnabled());
    dst->SetFogColor(src->GetFogColor());
    dst->SetFogDensity(src->GetFogDensity());

//    if(lightingParams)
//    {
//        dst->lightingParams = new StaticLightingParams();
//        dst->lightingParams->transparencyColor = lightingParams->transparencyColor;
//    }

    dst->SetAlphablend(src->GetAlphablend());
    dst->EnableFlatColor(src->IsFlatColorEnabled());
    
    dst->EnableTextureShift(src->IsTextureShiftEnabled());

    dst->SetWireframe(src->GetWireframe());
}

void DAEConvertWithSettingsAction::CopyLODSettings(DAVA::Scene * srcScene, DAVA::Scene * dstScene) const
{
    Vector<LodComponent *> srcLODs;
    EditorLODData::EnumerateLODsRecursive(srcScene, srcLODs);

    Vector<LodComponent *> dstLODs;
    EditorLODData::EnumerateLODsRecursive(dstScene, dstLODs);

    if(srcLODs.size() == dstLODs.size())
    {
        uint32 count = srcLODs.size();
        for (uint32 i = 0; i < count; ++i)
        {
            CopyLOD(srcLODs[i], dstLODs[i]);
        }
    }
}

void DAEConvertWithSettingsAction::CopyLOD(DAVA::LodComponent * src, DAVA::LodComponent * dst) const
{
    if(src->GetEntity()->GetName() != dst->GetEntity()->GetName())
        return;
    
	//Lod values
	for(int32 iLayer = 0; iLayer < LodComponent::MAX_LOD_LAYERS; ++iLayer)
	{
        dst->SetLodLayerDistance(iLayer, src->GetLodLayerDistance(iLayer));
	}
    
    dst->SetForceDistance(src->GetForceDistance());
    dst->SetForceLodLayer(src->GetForceLodLayer());
}


