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



#ifndef __RESOURCEEDITORQT__SETTINGS_MANAGER__
#define __RESOURCEEDITORQT__SETTINGS_MANAGER__

#include <QObject>
#include "DAVAEngine.h"
#include "FileSystem/VariantType.h"
#include "../StringConstants.h"

struct SettingRow
{
	DAVA::String key;
	DAVA::VariantType defValue;

	SettingRow(const char* _key, DAVA::VariantType& _defaultValue)
	{
		key = _key;
		defValue  = _defaultValue;
	}
};

static const SettingRow SETTINGS_GROUP_GENERAL_MAP[] = 
{
	SettingRow(ResourceEditor::SETTINGS_DESIGNER_NAME, DAVA::VariantType(DAVA::String("nobody"))),
	SettingRow(ResourceEditor::SETTINGS_PREVIEW_DIALOG_ENABLED, DAVA::VariantType(false)),
};

static const SettingRow SETTINGS_GROUP_DEFAULT_MAP[] = 
{
	SettingRow(ResourceEditor::SETTINGS_GRID_STEP, DAVA::VariantType(10.0f)),
	SettingRow("CameraSpeedValue_0", DAVA::VariantType(35.0f)),
	SettingRow("CameraSpeedValue_1", DAVA::VariantType(100.0f)),
	SettingRow("CameraSpeedValue_2", DAVA::VariantType(250.0f)),
	SettingRow("CameraSpeedValue_3", DAVA::VariantType(400.0f)),
	SettingRow(ResourceEditor::SETTINGS_DEFAULT_FOV, DAVA::VariantType(70.0f)),
	
};

static const SettingRow SETTINGS_GROUP_INTERNAL_MAP[] = 
{
	SettingRow(ResourceEditor::SETTINGS_3D_DATA_SOURCEPATH, DAVA::VariantType(DAVA::String("/"))),
	SettingRow(ResourceEditor::SETTINGS_PROJECT_PATH, DAVA::VariantType(DAVA::String(""))),
	SettingRow(ResourceEditor::SETTINGS_LAST_OPENED_FILES_COUNT, DAVA::VariantType(0)),//?!
	SettingRow(ResourceEditor::SETTINGS_TEXTURE_VIEW_GPU, DAVA::VariantType(GPU_UNKNOWN)),
	SettingRow("editor.version", DAVA::VariantType(DAVA::String("local build"))),
	SettingRow(ResourceEditor::SETTINGS_CUBEMAP_LAST_FACE_DIR, DAVA::VariantType(DAVA::String(""))),
	SettingRow(ResourceEditor::SETTINGS_CUBEMAP_LAST_PROJECT_DIR, DAVA::VariantType(DAVA::String(""))),
};

class SettingsManager: public DAVA::Singleton<SettingsManager>
{
public:

	static enum eSettingsGroups
	{
		GENERAL = 0,
		DEFAULT,
		INTERNAL,

		GROUPS_COUNT
	};
	
	SettingsManager();

	~SettingsManager();

	DAVA::KeyedArchive* GetSettingsGroup(eSettingsGroups group);

	DAVA::VariantType* GetValue(const DAVA::String& _name, eSettingsGroups group);

	void SetValue(const DAVA::String& _name, const DAVA::VariantType& _value, eSettingsGroups group);

	void SaveSettings();

	DAVA::String GetNameOfGroup(eSettingsGroups group);

	void Initialize();

private:

	DAVA::KeyedArchive* settings;

	void LoadSettings();

};
/*
class SettingsManager2: public QObject, public DAVA::Singleton<SettingsManager2>
{
	Q_OBJECT

public:
	SettingsManager2();
	virtual ~SettingsManager2();

	const DAVA::VariantType& GetValue(DAVA::FastName key);
	void SetValue(DAVA::FastName key, const DAVA::VariantType& value);

	void TrackSettings(DAVA::InspInfo *insp, void *object);

protected:
	struct SettingRow
	{
		void *object;
		DAVA::InspMember *member;
	};

	DAVA::FastNameMap<SettingRow> settings;
};
*/

#endif /* defined(__RESOURCEEDITORQT__SETTINGS_MANAGER__) */