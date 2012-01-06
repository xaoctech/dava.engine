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
#include "Scene3D/SceneFile2.h"
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
    
SceneFile2::SceneFile2()
{
    isDebugLogEnabled = false;
}

SceneFile2::~SceneFile2()
{
}
    
void SceneFile2::EnableDebugLog(bool _isDebugLogEnabled)
{
    isDebugLogEnabled = _isDebugLogEnabled;
}

bool SceneFile2::SaveScene(const char *filename, DAVA::Scene *_scene)
{
    File * file = File::Create(filename, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("SceneFile2::SaveScene failed to create file: %s", filename);
        return false;
    }   
    // save header
    Header header;
    header.signature[0] = 'S';
    header.signature[1] = 'F';
    header.signature[2] = 'V';
    header.signature[3] = '2';
    
    header.version = 1;
    header.nodeCount = _scene->GetChildrenCount();
    
    file->Write(&header, sizeof(Header));
    
    // save data objects
    Logger::Debug("+ save data objects");

    SaveHierarchy(_scene->GetMaterials(), file, 1);
    SaveHierarchy(_scene->GetStaticMeshes(), file, 1);
    
    // save hierarchy
    Logger::Debug("+ save hierarchy");

    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        if (!SaveHierarchy(_scene->GetChild(ci), file, 1))
        {
            Logger::Error("SceneFile2::SaveScene failed to save hierarchy file: %s", filename);
            SafeRelease(file);
            return false;
        }
    }
    
    SafeRelease(file);
    return true;
};	
    
bool SceneFile2::LoadScene(const char * filename, Scene * _scene)
{
    File * file = File::Create(filename, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("SceneFile2::LoadScene failed to create file: %s", filename);
        return false;
    }   
    
    Header header;
    file->Read(&header, sizeof(Header));
    int requiredVersion = 1;
    if (    (header.version != requiredVersion) 
        ||  (header.signature[0] != 'S') 
        ||  (header.signature[1] != 'F') 
        ||  (header.signature[2] != 'V') 
        ||  (header.signature[3] != '2'))
    {
        Logger::Error("SceneFile2::LoadScene header version is wrong: %d, required: %d", header.version, requiredVersion);
        
        SafeRelease(file);
        return false;
    }
    
    Logger::Debug("+ load data objects");

    LoadHierarchy(_scene, _scene->GetMaterials(), file, 1);
    LoadHierarchy(_scene, _scene->GetStaticMeshes(), file, 1);

    Logger::Debug("+ load hierarchy");
    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        LoadHierarchy(_scene, _scene, file, 1);
    }
    
    SafeRelease(file);
    return true;
}
    
bool SceneFile2::SaveHierarchy(DataNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    node->Save(archive);    
    
    if (isDebugLogEnabled)
        Logger::Debug("%s %s(%s)", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str());
    
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        DataNode * child = node->GetChild(ci);
        SaveHierarchy(child, file, level + 1);
    }
    
    SafeRelease(archive);
    return true;
}

void SceneFile2::LoadHierarchy(Scene * scene, DataNode * root, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    DataNode * node = dynamic_cast<DataNode*>(BaseObject::LoadFromArchive(archive));
    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            node = root;
        }   
        node->SetScene(scene);
        if (node != root)
            root->AddNode(node);
        if (isDebugLogEnabled)
            Logger::Debug("%s %s(%s)", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str());
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadHierarchy(scene, node, file, level + 1);
        }
    }
    
    SafeRelease(archive);
}
    
bool SceneFile2::SaveHierarchy(SceneNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    node->Save(archive);    
    
    archive->SetInt32("#childrenCount", node->GetChildrenCount());

    if (isDebugLogEnabled)
        Logger::Debug("%s %s(%s) %d", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str(), node->GetChildrenCount());
    
    archive->Save(file);

    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        SceneNode * child = node->GetChild(ci);
        SaveHierarchy(child, file, level + 1);
    }
    
    SafeRelease(archive);
    return true;
}

void SceneFile2::LoadHierarchy(Scene * scene, SceneNode * root, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    //SceneNode * node = dynamic_cast<SceneNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    SceneNode * node = dynamic_cast<SceneNode*>(ObjectFactory::Instance()->New(name));
    if (node)
    {
        node->SetScene(scene);
        node->Load(archive);
        root->AddNode(node);
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);

        if (isDebugLogEnabled)
            Logger::Debug("%s %s(%s) %d", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str(), childrenCount);

        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadHierarchy(scene, node, file, level + 1);
        }
    }else 
    {
    }
    
    SafeRelease(archive);
}
    
};