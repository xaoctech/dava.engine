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



#include "SettingsManager.h"
#include "Deprecated/ControlsFactory.h"
#include "Render/RenderManager.h"
#include "Scene/System/EditorMaterialSystem.h"
#include "TextureCompression/TextureConverter.h"

#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"
#include <QHeaderView>

#define SETTINGS_CONFIG_FILE "~doc:/ResourceEditorOptions.archive"

SettingsManager::SettingsManager()
{
	Init();
	Load();
}

SettingsManager::~SettingsManager()
{
	Save();
}

void SettingsManager::Init()
{
	CreateValue("General/DesignerName", DAVA::VariantType(DAVA::String("nobody")));
    CreateValue("General/RecentFilesCount", DAVA::VariantType(5));
	CreateValue("General/PreviewEnabled", DAVA::VariantType(false));
	CreateValue("General/CompressionQuality", DAVA::VariantType(DAVA::TextureConverter::ECQ_DEFAULT), DAVA::InspDesc("Compression quality", GlobalEnumMap<DAVA::TextureConverter::eConvertQuality>::Instance(), DAVA::InspDesc::T_ENUM));

    CreateValue("General/MaterialEditor/SwitchColor0", DAVA::VariantType(DAVA::Color(0.0f, 1.0f, 0.0f, 1.0f)));
    CreateValue("General/MaterialEditor/SwitchColor1", DAVA::VariantType(DAVA::Color(1.0f, 0.0f, 0.0f, 1.0f)));
    CreateValue("General/MaterialEditor/LodColor0", DAVA::VariantType(DAVA::Color(0.9f, 0.9f, 0.9f, 1.0f)));
    CreateValue("General/MaterialEditor/LodColor1", DAVA::VariantType(DAVA::Color(0.7f, 0.7f, 0.7f, 1.0f)));
    CreateValue("General/MaterialEditor/LodColor2", DAVA::VariantType(DAVA::Color(0.5f, 0.5f, 0.5f, 1.0f)));
    CreateValue("General/MaterialEditor/LodColor3", DAVA::VariantType(DAVA::Color(0.3f, 0.3f, 0.3f, 1.0f)));

	CreateValue("Scene/GridStep", DAVA::VariantType(10.0f));
	CreateValue("Scene/GridSize", DAVA::VariantType(600.0f));
	CreateValue("Scene/CameraSpeed0", DAVA::VariantType(35.0f));
	CreateValue("Scene/CameraSpeed1", DAVA::VariantType(100.0f));
	CreateValue("Scene/CameraSpeed2", DAVA::VariantType(250.0f));
	CreateValue("Scene/CameraSpeed3", DAVA::VariantType(400.0f));
	CreateValue("Scene/CameraFOV", DAVA::VariantType(70.0f));
    CreateValue("Scene/SelectionSequent", DAVA::VariantType(false));
    CreateValue("Scene/SelectionDrawMode", DAVA::VariantType(SceneSelectionSystem::DRAW_DEFAULT), DAVA::InspDesc("Selection draw modes", GlobalEnumMap<SceneSelectionSystem::DrawMode>::Instance(), DAVA::InspDesc::T_FLAGS));
    CreateValue("Scene/CollisionDrawMode", DAVA::VariantType(SceneCollisionSystem::DRAW_DEFAULT), DAVA::InspDesc("Collision draw modes", GlobalEnumMap<SceneCollisionSystem::DrawMode>::Instance(), DAVA::InspDesc::T_FLAGS));
    CreateValue("Scene/GizmoScale", DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue("Scene/DebugBoxScale", DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue("Scene/DebugBoxUserScale", DAVA::VariantType(DAVA::float32(1.0)));
    CreateValue("Scene/DebugBoxParticleScale", DAVA::VariantType(DAVA::float32(1.0)));

    CreateValue("Internal/TextureViewGPU", DAVA::VariantType(GPU_UNKNOWN));
	CreateValue("Internal/LastProjectPath", DAVA::VariantType(DAVA::FilePath()));
	CreateValue("Internal/EditorVersion", DAVA::VariantType(DAVA::String("local build")));
	CreateValue("Internal/CubemapLastFaceDir", DAVA::VariantType(DAVA::FilePath()));
	CreateValue("Internal/CubemapLastProjDir", DAVA::VariantType(DAVA::FilePath()));

	CreateValue("Internal/RecentFiles", DAVA::VariantType((KeyedArchive *) NULL));
    CreateValue("Internal/MaterialsLightViewMode", DAVA::VariantType(EditorMaterialSystem::LIGHTVIEW_ALL));
    CreateValue("Internal/MaterialsShowLightmapCanvas", DAVA::VariantType(false));
    CreateValue("Internal/LicenceAccepted", DAVA::VariantType(false));
}

DAVA::VariantType SettingsManager::GetValue(const DAVA::FastName& path)
{
    SettingsNode* node = SettingsManager::Instance()->GetPath(path, false);
     
    DVASSERT(NULL != node);
    DVASSERT(node->childs.size() == 0 && "Value can't be get from intermediate node");

    return node->value;
}

void SettingsManager::SetValue(const DAVA::FastName& path, const DAVA::VariantType &value)
{
    SettingsNode* node = SettingsManager::Instance()->GetPath(path, false);
    
    DVASSERT(NULL != node);
    DVASSERT(node->childs.size() == 0 && "Value can't be set into intermediate node");
    DVASSERT(node->value.type == value.type);

    node->value.SetVariant(value);
}

DAVA::VariantType SettingsManager::GetValue(const DAVA::String& path)
{
    return GetValue(DAVA::FastName(path));
}

void SettingsManager::SetValue(const DAVA::String& path, const DAVA::VariantType& value)
{
    SetValue(DAVA::FastName(path), value);
}

const SettingsNode* SettingsManager::GetSettingsTree()
{
    return &SettingsManager::Instance()->root;
}

void SettingsManager::Load()
{
	KeyedArchive* toLoad = new KeyedArchive();
	if(toLoad->Load(SETTINGS_CONFIG_FILE))
	{
        MergeSettingsIn(&root, toLoad);
	}

    toLoad->Release();
}

void SettingsManager::Save()
{
	KeyedArchive* toSave = new KeyedArchive();

    MergeSettingsOut(toSave, &root);

	toSave->Save(SETTINGS_CONFIG_FILE);
    toSave->Release();
}

/*
const EnumMap * SettingsManager::GetAllowedValues(const DAVA::String& _name, eSettingsGroups group) const
{
    SettingRow * settingMap = NULL;
    DAVA::uint32 settingsCount = 0;
    
    switch (group)
    {
        case GENERAL:
            settingMap = (SettingRow *)SETTINGS_GROUP_GENERAL_MAP;
            settingsCount = COUNT_OF(SETTINGS_GROUP_GENERAL_MAP);
            break;

        case DEFAULT:
            settingMap = (SettingRow *)SETTINGS_GROUP_DEFAULT_MAP;
            settingsCount = COUNT_OF(SETTINGS_GROUP_DEFAULT_MAP);
            break;

        case INTERNAL:
            settingMap = (SettingRow *)SETTINGS_GROUP_INTERNAL_MAP;
            settingsCount = COUNT_OF(SETTINGS_GROUP_INTERNAL_MAP);
            break;

        default:
            DVASSERT(false);
            return NULL;
    }

    for(DAVA::uint32 i = 0; i < settingsCount; ++i)
    {
        if(settingMap[i].key == _name)
        {
            return settingMap[i].enumMap;
        }
    }
    
    return NULL;
}


void SettingsManager::SetValue(const DAVA::String& _name, const DAVA::VariantType& _value, eSettingsGroups group)
*/

void SettingsManager::MergeSettingsIn(SettingsNode *target, DAVA::KeyedArchive *source)
{
    for(size_t i = 0; i < target->childs.size(); ++i)
    {
        SettingsNode *node = target->childs[i];
        DAVA::String nodeName = node->name.c_str();

        if(source->IsKeyExists(nodeName))
        {
            DAVA::VariantType* sourceValue = source->GetVariant(nodeName);

            // if this node hasn't child try to load
            // value from source
            if(node->childs.size() == 0)
            {
                if(sourceValue->type == node->value.type)
                {
                    node->value.SetVariant(*sourceValue);
                }
            }
            // this is intermediate node, try to load it from source
            // (can be done only if source variant type is KeyedArchive)
            else
            {
                if(sourceValue->type == DAVA::VariantType::TYPE_KEYED_ARCHIVE)
                {
                    MergeSettingsIn(node, sourceValue->AsKeyedArchive());
                }
            }
        }
    }
}

void SettingsManager::MergeSettingsOut(DAVA::KeyedArchive *target, SettingsNode *source)
{
    for(size_t i = 0; i < source->childs.size(); ++i)
    {
        SettingsNode *node = source->childs[i];
        DAVA::String nodeName = node->name.c_str();
        DAVA::VariantType nodeValue = node->value;

        if(0 != node->childs.size())
        {
            DAVA::KeyedArchive *intermediateArchive = new DAVA::KeyedArchive();

            MergeSettingsOut(intermediateArchive, node);

            nodeValue.SetKeyedArchive(intermediateArchive);
            intermediateArchive->Release();
        }

        target->SetVariant(nodeName, nodeValue);
    }
}

void SettingsManager::CreateValue(const DAVA::String& path, const DAVA::VariantType &defaultValue, DAVA::InspDesc description)
{
    DAVA::FastName pathName = DAVA::FastName(path);
    SettingsNode* node = GetPath(pathName, true);
    
    DVASSERT(NULL != node);
    DVASSERT(node->childs.size() == 0 && "Value can't be set into node, that has childs");

    node->value = defaultValue;
    node->path = DAVA::FastName(path);
    node->description = description;
}

SettingsNode* SettingsManager::GetPath(const DAVA::FastName& path, bool create)
{
    SettingsNode *ret = NULL;

    DAVA::String key;
    DAVA::Vector<DAVA::FastName> keys;

    std::stringstream ss(path.c_str());
    while(std::getline(ss, key, '/'))
    {
        keys.push_back(DAVA::FastName(key));
    }

    if(keys.size() > 0)
    {
        SettingsNode *parent = &root;

        // we should go deep into structure to find
        // requested path
        for(size_t i = 0; i < keys.size(); ++i)
        {
            SettingsNode *node = parent->GetChild(keys[i]);
            if(NULL == node && create)
            {
                DAVA::String createdPath = "";

                if(parent->path.IsValid())
                {
                    createdPath += DAVA::String(parent->path.c_str());
                    createdPath += DAVA::String("/");
                }

                createdPath += DAVA::String(keys[i].c_str());

                node = new SettingsNode(keys[i]);
                node->path = DAVA::FastName(createdPath);
                parent->childs.push_back(node);
            }

            parent = node;
            if(NULL == parent)
            {
                break;
            }
        }

        ret = parent;
    }

    return ret;
}

SettingsNode::SettingsNode(const DAVA::FastName &_name)
: name(_name)
{ }

SettingsNode::~SettingsNode()
{
    for(size_t i = 0; i < childs.size(); ++i)
    {
        delete childs[i];
    }
}

SettingsNode* SettingsNode::GetChild(const DAVA::FastName &name)
{
    SettingsNode* ret = NULL;

    for(size_t i = 0; i < childs.size(); ++i)
    {
        if(childs[i]->name == name)
        {
            ret = childs[i];
            break;
        }
    }

    return ret;
}
