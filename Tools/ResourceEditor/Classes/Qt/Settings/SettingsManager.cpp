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
#include <QHeaderView>

#define CONFIG_FILE				"~doc:/ResourceEditorOptions.archive"
#define SETTINGS_VERSION_NUMBER	1
#define SETTINGS_VERSION_KEY	"SETTINGS_VERSION"

SettingsManager::SettingsManager()
{
	settings = new KeyedArchive();

	Initialize();
}

SettingsManager::~SettingsManager()
{
	settings->DeleteAllKeys();
	SafeRelease(settings);
}

DAVA::VariantType* SettingsManager::GetValue(const DAVA::String& _name, eSettingsGroups _group)
{
	KeyedArchive* foundedGroupSettings = settings->GetArchive(GetNameOfGroup(_group));
	DVASSERT(foundedGroupSettings->IsKeyExists(_name));
	return foundedGroupSettings->GetVariant(_name);
}

void SettingsManager::SetValue(const DAVA::String& _name, const DAVA::VariantType& _value, eSettingsGroups group)
{
	KeyedArchive* foundedGroupSettings = settings->GetArchive(GetNameOfGroup(group));
	DVASSERT(foundedGroupSettings->IsKeyExists(_name));
	foundedGroupSettings->SetVariant(_name, _value);
}

void SettingsManager::Initialize()
{
	settings->DeleteAllKeys();
	KeyedArchive* generalSettings = new KeyedArchive();
	KeyedArchive* defaultSettings = new KeyedArchive();
	KeyedArchive* internalSettings = new KeyedArchive();
	
	for (uint32 i = 0; i < (sizeof(SETTINGS_GROUP_GENERAL_MAP) / sizeof(SettingRow)); ++i )
	{
		generalSettings->SetVariant( SETTINGS_GROUP_GENERAL_MAP[i].key, DAVA::VariantType(SETTINGS_GROUP_GENERAL_MAP[i].defValue) );
	}

	for (uint32 i = 0; i < (sizeof(SETTINGS_GROUP_DEFAULT_MAP) / sizeof(SettingRow)); ++i )
	{
		defaultSettings->SetVariant( SETTINGS_GROUP_DEFAULT_MAP[i].key, DAVA::VariantType(SETTINGS_GROUP_DEFAULT_MAP[i].defValue) );
	}

	for (uint32 i = 0; i < (sizeof(SETTINGS_GROUP_INTERNAL_MAP) / sizeof(SettingRow)); ++i )
	{
		internalSettings->SetVariant( SETTINGS_GROUP_INTERNAL_MAP[i].key, DAVA::VariantType(SETTINGS_GROUP_INTERNAL_MAP[i].defValue) );
	}
	settings->SetArchive(GetNameOfGroup(GENERAL), generalSettings);
	settings->SetArchive(GetNameOfGroup(DEFAULT), defaultSettings);
	settings->SetArchive(GetNameOfGroup(INTERNAL), internalSettings);
	settings->SetUInt32(SETTINGS_VERSION_KEY, SETTINGS_VERSION_NUMBER);

	LoadSettings();
}

void SettingsManager::LoadSettings()
{
	KeyedArchive* loadedArchive = new KeyedArchive();
	bool success = loadedArchive->Load(CONFIG_FILE);
	DVASSERT(success);

	bool isSettingsFileVersionActual = loadedArchive->IsKeyExists(SETTINGS_VERSION_KEY);
	// perform synchronization: update present in settings elements from file
	for (uint32 i = 0; i < GROUPS_COUNT; ++i)
	{
		KeyedArchive* presentSettingsGroup = GetSettingsGroup((eSettingsGroups)i);
		KeyedArchive* archiveFromFile = isSettingsFileVersionActual ? 
			 loadedArchive->GetArchive(GetNameOfGroup((eSettingsGroups)i)) : loadedArchive;
	
		if(NULL == archiveFromFile)
		{
			continue;
		}

		Map<String, VariantType*> predefinedSettingsGroupMap = presentSettingsGroup->GetArchieveData();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator it = predefinedSettingsGroupMap.begin();
		DAVA::Map<DAVA::String, DAVA::VariantType*>::iterator end = predefinedSettingsGroupMap.end();
	
		for(; it != end; ++it)
		{
			if(archiveFromFile->IsKeyExists(it->first))
			{
				presentSettingsGroup->SetVariant(it->first, *archiveFromFile->GetVariant(it->first));
			}
		}	
	}

	SafeRelease(loadedArchive);
}

void SettingsManager::SaveSettings()
{
	settings->Save(CONFIG_FILE);
}

DAVA::KeyedArchive* SettingsManager::GetSettingsGroup(eSettingsGroups group)
{
	String nameOfGroup = GetNameOfGroup(group);
	return settings->GetArchive(nameOfGroup);
}

DAVA::String SettingsManager::GetNameOfGroup(eSettingsGroups group)
{
	//todo wrap into map
	String retValue;
	switch (group)
	{
	case SettingsManager::GENERAL:
		retValue = "General";
		break;
	case SettingsManager::DEFAULT:
		retValue = "Scene default";
		break;
	case SettingsManager::INTERNAL:
		retValue = "Internal";
		break;
	default:
		DVASSERT(0);
		break;
	}
	return retValue;
}
