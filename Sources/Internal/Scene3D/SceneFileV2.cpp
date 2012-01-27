/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/3D/AnimatedMesh.h"
#include "Scene3D/PathManip.h"
#include "Scene3D/SkeletonNode.h"
#include "Scene3D/BoneNode.h"
#include "Scene3D/Camera.h"
#include "Scene3D/SceneNodeAnimationList.h"
#include "Utils/StringFormat.h"
#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"

namespace DAVA
{
    
SceneFileV2::SceneFileV2()
{
    isDebugLogEnabled = false;
    isSaveForGame = false;
}

SceneFileV2::~SceneFileV2()
{
}
    
const String & SceneFileV2::GetScenePath()
{
    return rootNodePath;
}
    
const String & SceneFileV2::GetSceneFilename()
{
    return rootNodeName;
}
    
static void replace(std::string & repString,const std::string & needle, const std::string & s)
{
    std::string::size_type lastpos = 0, thispos;
    while ((thispos = repString.find(needle, lastpos)) != std::string::npos)
    {
        repString.replace(thispos, needle.length(), s);
        lastpos = thispos + 1;
    }
}
    
void SceneFileV2::EnableSaveForGame(bool _isSaveForGame)
{
    isSaveForGame = _isSaveForGame;
}

String SceneFileV2::AbsoluteToRelative(const String & absolutePathname)
{
    String result = absolutePathname;
    
    if (isSaveForGame)
    {
        size_t pos = result.find("DataSource");
        if (pos != result.npos)
        {
            result.replace(pos, strlen("DataSource"), "Data");
        }
    }

//    replace(result, GetScenePath(), String(""));
    result = FileSystem::AbsoluteToRelativePath(GetScenePath(), result);
    return result;
}
    
String SceneFileV2::RelativeToAbsolute(const String & relativePathname)
{
    String result;
    result = GetScenePath() + relativePathname;
    return result;
}
    
void SceneFileV2::EnableDebugLog(bool _isDebugLogEnabled)
{
    isDebugLogEnabled = _isDebugLogEnabled;
}
    
Material * SceneFileV2::GetMaterial(int32 index)
{
    return materials[index];
}
    
StaticMesh * SceneFileV2::GetStaticMesh(int32 index)
{
    return staticMeshes[index];
}

DataNode * SceneFileV2::GetNodeByPointer(uint64 pointer)
{
    Map<uint64, DataNode*>::iterator it = dataNodes.find(pointer);
    if (it != dataNodes.end())
    {
        return it->second;
    }
    return 0;
}

int32 SceneFileV2::GetVersion()
{
    return header.version;
}

bool SceneFileV2::SaveScene(const String & filename, DAVA::Scene *_scene)
{
    File * file = File::Create(filename, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("SceneFileV2::SaveScene failed to create file: %s", filename.c_str());
        return false;
    }
    
    rootNodePathName = filename;
    FileSystem::Instance()->SplitPath(rootNodePathName, rootNodePath, rootNodeName);

    // save header
    header.signature[0] = 'S';
    header.signature[1] = 'F';
    header.signature[2] = 'V';
    header.signature[3] = '2';
    
    header.version = 2;
    header.nodeCount = _scene->GetChildrenCount();
    
    file->Write(&header, sizeof(Header));
    
    // save data objects
    Logger::Debug("+ save data objects");
    Logger::Debug("- save file path: %s", rootNodePath.c_str());
    
//    // Process file paths
//    for (int32 mi = 0; mi < _scene->GetMaterials()->GetChildrenCount(); ++mi)
//    {
//        Material * material = dynamic_cast<Material*>(_scene->GetMaterials()->GetChild(mi));
//        for (int k = 0; k < Material::TEXTURE_COUNT; ++k)
//        {
//            if (material->names[k].length() > 0)
//            {
//                replace(material->names[k], rootNodePath, String(""));
//                Logger::Debug("- preprocess mat path: %s rpn: %s", material->names[k].c_str(), material->textures[k]->relativePathname.c_str());
//            }
//        }   
//    }
    
//    SaveDataHierarchy(_scene->GetMaterials(), file, 1);
//    SaveDataHierarchy(_scene->GetStaticMeshes(), file, 1);

    List<DataNode*> nodes;
    _scene->GetDataNodes(nodes);
    int32 dataNodesCount = (int32)nodes.size();
    file->Write(&dataNodesCount, sizeof(int32));
    for (List<DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        SaveDataNode(*it, file);
    
    // save hierarchy
    Logger::Debug("+ save hierarchy");

    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        if (!SaveHierarchy(_scene->GetChild(ci), file, 1))
        {
            Logger::Error("SceneFileV2::SaveScene failed to save hierarchy file: %s", filename.c_str());
            SafeRelease(file);
            return false;
        }
    }
    
    SafeRelease(file);
    return true;
};	
    
bool SceneFileV2::LoadScene(const String & filename, Scene * _scene)
{
    File * file = File::Create(filename, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("SceneFileV2::LoadScene failed to create file: %s", filename.c_str());
        return false;
    }   

    scene = _scene;
    rootNodePathName = filename;
    FileSystem::Instance()->SplitPath(rootNodePathName, rootNodePath, rootNodeName);

    file->Read(&header, sizeof(Header));
    int requiredVersion = 1;
    if (   
          (header.signature[0] != 'S') 
        ||  (header.signature[1] != 'F') 
        ||  (header.signature[2] != 'V') 
        ||  (header.signature[3] != '2'))
    {
        Logger::Error("SceneFileV2::LoadScene header version is wrong: %d, required: %d", header.version, requiredVersion);
        
        SafeRelease(file);
        return false;
    }
    
    Logger::Debug("+ load data objects");

    if (GetVersion() == 1)
    {
        DataNode * materialsTemp = new DataNode;
        LoadDataHierarchy(_scene, materialsTemp, file, 1);
        SafeRelease(materialsTemp);
        DataNode * staticMeshes = new DataNode;
        LoadDataHierarchy(_scene, staticMeshes, file, 1);
        SafeRelease(staticMeshes);
    }else if (GetVersion() == 2)
    {
        int32 dataNodeCount = 0;
        file->Read(&dataNodeCount, sizeof(int32));
        
        for (int k = 0; k < dataNodeCount; ++k)
            LoadDataNode(0, file);
    }
    
    Logger::Debug("+ load hierarchy");
    
    SceneNode * rootNode = new SceneNode(_scene);
    rootNode->SetName(rootNodeName);
    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        LoadHierarchy(_scene, rootNode, file, 1);
    }
    rootNode->AddFlagRecursive(SceneNode::NODE_VISIBLE);
	rootNode->SceneDidLoaded();
    _scene->AddRootNode(rootNode, rootNodePathName);
    
    
    for (size_t mi = 0; mi < materials.size(); ++mi)
    {
        SafeRelease(materials[mi]);
    }
    materials.clear();
    
    for (size_t mi = 0; mi < staticMeshes.size(); ++mi)
    {
        SafeRelease(staticMeshes[mi]);
    }
    staticMeshes.clear();
    
    for (Map<uint64, DataNode*>::iterator it = dataNodes.begin(); it != dataNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    dataNodes.clear();
    
    SafeRelease(file);
    return true;
}

bool SceneFileV2::SaveDataNode(DataNode * node, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::Debug("- %s(%s)", node->GetName().c_str(), node->GetClassName().c_str());
    
    
    node->Save(archive, this);  
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        DataNode * child = node->GetChild(ci);
        SaveDataNode(child, file);
    }
    
    SafeRelease(archive);
    return true;
}
    
void SceneFileV2::LoadDataNode(DataNode * parent, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New(name));
    
    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            return;
        }   
        node->SetScene(scene);
        
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::Debug("- %s(%s)", name.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, this);
        AddToNodeMap(node);
        
        if (parent)
            parent->AddNode(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadDataNode(node, file);
        }
        
        SafeRelease(node);
    }
    SafeRelease(archive);
}

bool SceneFileV2::SaveDataHierarchy(DataNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::Debug("%s %s(%s)", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str());

    node->Save(archive, this);    
    
    
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        DataNode * child = node->GetChild(ci);
        SaveDataHierarchy(child, file, level + 1);
    }
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadDataHierarchy(Scene * scene, DataNode * root, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    // DataNode * node = dynamic_cast<DataNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New(name));

    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            node = SafeRetain(root); // retain root here because we release it at the end
        }  
        
        node->SetScene(scene);
        
        // TODO: Rethink here
        Material * material = dynamic_cast<Material*>(node);
        if (material)
        {
            materials.push_back(SafeRetain(material));
        }
        StaticMesh * staticMesh = dynamic_cast<StaticMesh*>(node);
        if (staticMesh)
        {
            staticMeshes.push_back(SafeRetain(staticMesh));
        }
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::Debug("%s %s(%s)", GetIndentString('-', level), name.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, this);
        
        
        AddToNodeMap(node);
        
        if (node != root)
            root->AddNode(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadDataHierarchy(scene, node, file, level + 1);
        }
        SafeRelease(node);
    }
    
    SafeRelease(archive);
}
    
void SceneFileV2::AddToNodeMap(DataNode * node)
{
    uint64 ptr = node->GetPreviousPointer();
    Logger::Debug("* add ptr: %llx class: %s(%s)", ptr, node->GetName().c_str(), node->GetClassName().c_str());
    dataNodes[ptr] = SafeRetain(node);
}
    
bool SceneFileV2::SaveHierarchy(SceneNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::Debug("%s %s(%s) %d", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str(), node->GetChildrenCount());
    node->Save(archive, this);    
    
    archive->SetInt32("#childrenCount", node->GetChildrenCount());

    
    archive->Save(file);

    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        SceneNode * child = node->GetChild(ci);
        SaveHierarchy(child, file, level + 1);
    }
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadHierarchy(Scene * scene, SceneNode * parent, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    //SceneNode * node = dynamic_cast<SceneNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    SceneNode * node = dynamic_cast<SceneNode*>(ObjectFactory::Instance()->New(name));
    if (node)
    {
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::Debug("%s %s(%s)", GetIndentString('-', level), name.c_str(), node->GetClassName().c_str());
        }

        node->SetScene(scene);
        node->Load(archive, this);
        parent->AddNode(node);
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);


        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadHierarchy(scene, node, file, level + 1);
        }
    }
    
    SafeRelease(archive);
}
    
};