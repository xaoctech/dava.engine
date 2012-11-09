#include "SceneExporter.h"
#include "SceneValidator.h"

using namespace DAVA;

SceneExporter::SceneExporter()
{
    dataFolder = String("");
    dataSourceFolder = String(""); 
    workingFolder = String("");
    
    SetExportingFormat(String("png"));
}

SceneExporter::~SceneExporter()
{
    
}

void SceneExporter::CleanFolder(const String &folderPathname, Set<String> &errorLog)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if(!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if(folderExists)
        {
            errorLog.insert(String(Format("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.c_str())));
        }
    }
}

void SceneExporter::SetInFolder(const String &folderPathname)
{
    dataSourceFolder = NormalizeFolderPath(folderPathname);
}

void SceneExporter::SetOutFolder(const String &folderPathname)
{
    dataFolder = NormalizeFolderPath(folderPathname);
}

void SceneExporter::SetExportingFormat(const String &newFormat)
{
    format = newFormat;
    if(0 < format.length() && format.at(0) != '.')
    {
        format = "." + format;
    }
}



void SceneExporter::ExportFile(const String &fileName, Set<String> &errorLog)
{
    //Load scene with *.sc2
    Scene *scene = new Scene();
    SceneNode *rootNode = scene->GetRootNode(dataSourceFolder + fileName);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
		Vector<SceneNode*> tempV;
		tempV.reserve((count));
        for(int32 i = 0; i < count; ++i)
        {
			tempV.push_back(rootNode->GetChild(i));
        }
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}
    }
    
    ExportScene(scene, fileName, errorLog);

    SafeRelease(scene);
}

void SceneExporter::ExportScene(Scene *scene, const String &fileName, Set<String> &errorLog)
{
    //Create destination folder
    String normalizedFileName = FileSystem::Instance()->GetCanonicalPath(fileName);

    
    String workingFile;
    FileSystem::SplitPath(normalizedFileName, workingFolder, workingFile);
    FileSystem::Instance()->CreateDirectory(dataFolder + workingFolder, true); 
    
    //Export scene data
    RemoveEditorNodes(scene);

    String oldPath = SceneValidator::Instance()->SetPathForChecking(dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, errorLog);
	//SceneValidator::Instance()->ValidateScales(scene, errorLog);

    ExportMaterials(scene, errorLog);
    ExportLandscape(scene, errorLog);
    ExportMeshLightmaps(scene, errorLog);
    
    //save scene to new place
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(true);
    outFile->EnableDebugLog(false);
    outFile->SaveScene(dataFolder + fileName, scene);
    SafeRelease(outFile);
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}



void SceneExporter::RemoveEditorNodes(DAVA::SceneNode *rootNode)
{
    //Remove scene nodes
    Vector<SceneNode *> scenenodes;
    rootNode->GetChildNodes(scenenodes);
        
    //remove nodes from hierarhy
    Vector<SceneNode *>::reverse_iterator endItDeletion = scenenodes.rend();
    for (Vector<SceneNode *>::reverse_iterator it = scenenodes.rbegin(); it != endItDeletion; ++it)
    {
        SceneNode * node = *it;
		String::size_type pos = node->GetName().find(String("editor."));
        if(String::npos != pos)
        {
            node->GetParent()->RemoveNode(node);
        }
		else
		{
			LightNode * light = dynamic_cast<LightNode*>(node);
			if(light)
			{
				bool isDynamic = light->GetCustomProperties()->GetBool("editor.dynamiclight.enable", true);
				if(!isDynamic)
				{
					node->GetParent()->RemoveNode(node);
				}
			}
		}
    }
}


void SceneExporter::ExportFolder(const String &folderName, Set<String> &errorLog)
{
    String folderPathname = dataSourceFolder + NormalizeFolderPath(folderName);
	FileList * fileList = new FileList(folderPathname);
    for (int32 i = 0; i < fileList->GetCount(); ++i)
	{
        String pathname = fileList->GetPathname(i);
		if(fileList->IsDirectory(i))
		{
            String curFolderName = fileList->GetFilename(i);
            if((String(".") != curFolderName) && (String("..") != curFolderName))
            {
                String workingPathname = RemoveFolderFromPath(pathname, dataSourceFolder);
                ExportFolder(workingPathname, errorLog);
            }
        }
        else 
        {
            String extension = FileSystem::Instance()->GetExtension(pathname);
            if(String(".sc2") == extension)
            {
                String workingPathname = RemoveFolderFromPath(pathname, dataSourceFolder);
                ExportFile(workingPathname, errorLog);
            }
        }
    }
    
    SafeRelease(fileList);
}

String SceneExporter::NormalizeFolderPath(const String &pathname)
{
    String normalizedPathname = FileSystem::Instance()->GetCanonicalPath(pathname);

    int32 lastPos = normalizedPathname.length() - 1;
    if((0 <= lastPos) && ('/' != normalizedPathname.at(lastPos)))
    {
        normalizedPathname += "/";
    }
    
    return normalizedPathname;
}


void SceneExporter::ExportMaterials(Scene *scene, Set<String> &errorLog)
{
    Logger::Debug("[ExportMaterials]");

    Vector<Material*> materials;
    scene->GetDataNodes(materials);
    for (int32 i = 0; i < (int32)materials.size(); i++)
    {
        Material *m = materials[i];
        if (m->GetName().find("editor.") == String::npos)
        {
			if (m->GetTexture(Material::TEXTURE_DIFFUSE))
			{
				if (!m->GetTexture(Material::TEXTURE_DIFFUSE)->relativePathname.empty())
				{
//                    m->SetTexture(Material::TEXTURE_DIFFUSE, ExportTexture(m->GetTextureName(Material::TEXTURE_DIFFUSE), errorLog));
                    m->SetTexture(Material::TEXTURE_DIFFUSE, ExportTexture(m->GetTexture(Material::TEXTURE_DIFFUSE)->relativePathname, errorLog));
				}
			}
            
			if (m->GetTexture(Material::TEXTURE_DECAL))
			{
				if (!m->GetTexture(Material::TEXTURE_DECAL)->relativePathname.empty())
				{
//                    m->SetTexture(Material::TEXTURE_DECAL, ExportTexture(m->GetTextureName(Material::TEXTURE_DECAL), errorLog));
                    m->SetTexture(Material::TEXTURE_DECAL, ExportTexture(m->GetTexture(Material::TEXTURE_DECAL)->relativePathname, errorLog));
				}
			}
        }
    }
}

void SceneExporter::ExportLandscape(Scene *scene, Set<String> &errorLog)
{
    Logger::Debug("[ExportLandscape]");

    Vector<LandscapeNode *> landscapes;
    scene->GetChildNodes(landscapes);
    
    if(0 < landscapes.size())
    {
        if(1 < landscapes.size())
        {
            errorLog.insert(String("There are more than one landscapes at level."));
        }
        
        LandscapeNode *landscape = landscapes[0];
        ExportFileDirectly(landscape->GetHeightmapPathname(), errorLog);
        
        for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; i++)
        {
            if(LandscapeNode::TEXTURE_DETAIL == i)
            {
                continue;
            }
            else if(LandscapeNode::TEXTURE_TILE_FULL == i)
            {
                ExportLandscapeFullTiledTexture(landscape, errorLog);
            }
            else
            {
                Texture *t = landscape->GetTexture((LandscapeNode::eTextureLevel)i);
                if(t)
                {
                    landscape->SetTextureName((LandscapeNode::eTextureLevel)i,
                                              ExportTexture(landscape->GetTextureName((LandscapeNode::eTextureLevel)i), errorLog));
                }
                else
                {
                    errorLog.insert(String(Format("There is no landscape texture for index %d", i)));
                }
            }
        }
    }
}

void SceneExporter::ExportLandscapeFullTiledTexture(LandscapeNode *landscape, Set<String> &errorLog)
{
    String textureName = landscape->GetTextureName(LandscapeNode::TEXTURE_TILE_FULL);
    if(0 < textureName.length())
    {
        landscape->SetTextureName(LandscapeNode::TEXTURE_TILE_FULL, ExportTexture(textureName, errorLog));
    }
    else
    {
        String colorTextureMame = landscape->GetTextureName(LandscapeNode::TEXTURE_COLOR);
        String filename, pathname;
        FileSystem::Instance()->SplitPath(colorTextureMame, pathname, filename);
        
        String fullTiledPathname = pathname + FileSystem::Instance()->ReplaceExtension(filename, ".thumbnail.png");
        String workingPathname = RemoveFolderFromPath(fullTiledPathname, dataSourceFolder);
        PrepareFolderForCopy(workingPathname, errorLog);

        Texture *fullTiledTexture = Texture::GetPinkPlaceholder();
        Image *image = fullTiledTexture->CreateImageFromMemory();
        if(image)
        {
            image->Save(dataFolder + workingPathname);
            SafeRelease(image);
        }
        else
        {
            errorLog.insert(String(Format("Can't create image for fullTiled Texture for file %s", workingPathname.c_str())));
        }
        
        landscape->SetTextureName(LandscapeNode::TEXTURE_TILE_FULL, dataSourceFolder + workingPathname);
    }
}


void SceneExporter::ExportMeshLightmaps(Scene *scene, Set<String> &errorLog)
{
    Logger::Debug("[ExportMeshLightmaps]");

    Vector<MeshInstanceNode *> meshInstances;
    scene->GetChildNodes(meshInstances);

    for(int32 iMesh = 0; iMesh < (int32)meshInstances.size(); ++iMesh)
    {
        for (int32 li = 0; li < meshInstances[iMesh]->GetLightmapCount(); ++li)
        {
            MeshInstanceNode::LightmapData * ld = meshInstances[iMesh]->GetLightmapDataForIndex(li);
            if (ld)
            {
                ld->lightmapName = ExportTexture(ld->lightmapName, errorLog);
            }
        }
    }
}


bool SceneExporter::ExportFileDirectly(const String &filePathname, Set<String> &errorLog)
{
    //    Logger::Debug("[ExportFileDirectly] %s", filePathname.c_str());

    String workingPathname = RemoveFolderFromPath(filePathname, dataSourceFolder);
    PrepareFolderForCopy(workingPathname, errorLog);
    
    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if(!retCopy)
    {
        errorLog.insert(String(Format("Can't copy %s from %s to %s", workingPathname.c_str(), dataSourceFolder.c_str(), dataFolder.c_str())));
    }
    
    return retCopy;
}

void SceneExporter::PrepareFolderForCopy(const String &filePathname, Set<String> &errorLog)
{
    String folder, file;
    FileSystem::SplitPath(filePathname, folder, file);
    
    String newFolderPath = dataFolder + folder;
    if(!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if(FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            errorLog.insert(String(Format("Can't create folder %s", newFolderPath.c_str())));
        }
    }
    
    FileSystem::Instance()->DeleteFile(dataFolder + filePathname);
}



String SceneExporter::ExportTexture(const String &texturePathname, Set<String> &errorLog)
{
		//    Logger::Debug("[ExportTexture] %s", texturePathname.c_str());
    
    //TODO: return at format will be enabled
    String exportedPathname = FileSystem::Instance()->ReplaceExtension(texturePathname, format);
    bool ret = ExportFileDirectly(exportedPathname, errorLog);
    if(!ret)
    {
        //TODO: blen textures
        RenderManager::Instance()->LockNonMain();

        Texture *tex = Texture::CreateFromFile(texturePathname);
        if(tex)
        {
            Sprite *fbo = Sprite::CreateAsRenderTarget((float32)tex->width, (float32)tex->height, FORMAT_RGBA8888);
            Sprite *texSprite = Sprite::CreateFromTexture(tex, 0, 0, (float32)tex->width, (float32)tex->height);
            if(fbo && texSprite)
            {
                
                RenderManager::Instance()->SetRenderTarget(fbo);
                texSprite->SetPosition(0.f, 0.f);
                texSprite->Draw();
                
                RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 1.0f, 0.5f));
                RenderHelper::Instance()->FillRect(Rect(0.0f, 0.0f, (float32)tex->width, (float32)tex->height));

                RenderManager::Instance()->RestoreRenderTarget();
                
                //Save new texture
                String workingPathname = RemoveFolderFromPath(texturePathname, dataSourceFolder);
                PrepareFolderForCopy(workingPathname, errorLog);
                exportedPathname = dataFolder + workingPathname;
                
                Image *image = fbo->GetTexture()->CreateImageFromMemory();
                if(image)
                {
                    image->Save(exportedPathname);
                }
                else 
                {
                    errorLog.insert(String(Format("Can't create image from fbo for file %s", texturePathname.c_str())));
                }
                
                SafeRelease(image);
            }
            else 
            {
                errorLog.insert(String(Format("Can't create FBO for file %s", texturePathname.c_str())));
            }
            
            SafeRelease(texSprite);
            SafeRelease(fbo);
            SafeRelease(tex);
        }
        else 
        {
            errorLog.insert(String(Format("Can't create texture for file %s", texturePathname.c_str())));
        }
        RenderManager::Instance()->UnlockNonMain();
        
    }
    
    return exportedPathname;
}


String SceneExporter::RemoveFolderFromPath(const String &pathname, const String &folderPathname)
{
    String workingPathname = pathname;
    String::size_type pos = workingPathname.find(folderPathname);
    if(String::npos != pos)
    {
        workingPathname = workingPathname.replace(pos, folderPathname.length(), "");
    }

    return workingPathname;
}


