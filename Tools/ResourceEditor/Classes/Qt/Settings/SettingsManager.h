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

	SettingRow(const char* _key,const DAVA::VariantType& _defaultValue)
	{
		key = _key;
		defValue  = _defaultValue;
	}
};

static const SettingRow SETTINGS_GROUP_GENERAL_MAP[] = 
{
	SettingRow("DesignerName", DAVA::VariantType(DAVA::String("nobody"))),
	SettingRow("PreviewDialogEnabled", DAVA::VariantType(false)),
};

static const SettingRow SETTINGS_GROUP_DEFAULT_MAP[] = 
{
	SettingRow("GridStep", DAVA::VariantType(10.0f)),
	SettingRow("CameraSpeedValue_0", DAVA::VariantType(35.0f)),
	SettingRow("CameraSpeedValue_1", DAVA::VariantType(100.0f)),
	SettingRow("CameraSpeedValue_2", DAVA::VariantType(250.0f)),
	SettingRow("CameraSpeedValue_3", DAVA::VariantType(400.0f)),
	SettingRow("DefaultCameraFOV", DAVA::VariantType(70.0f)),
	
};

static const SettingRow SETTINGS_GROUP_INTERNAL_MAP[] = 
{
	SettingRow("3dDataSourcePath", DAVA::VariantType(DAVA::String("/"))),
	SettingRow("ProjectPath", DAVA::VariantType(DAVA::String(""))),
	SettingRow("TextureViewGPU", DAVA::VariantType(GPU_UNKNOWN)),
	SettingRow("editor.version", DAVA::VariantType(DAVA::String("local build"))),
	SettingRow("cubemap_last_face_dir", DAVA::VariantType(DAVA::String(""))),
	SettingRow("cubemap_last_proj_dir", DAVA::VariantType(DAVA::String(""))),
};

static const SettingRow SETTINGS_GROUP_MUTABLE_LENGHT_MAP[] =
{
	SettingRow("LastOpenedFile", DAVA::VariantType(int32(0))),
};

class SettingsManager: public DAVA::Singleton<SettingsManager>
{
public:

	enum eSettingsGroups
	{
		GENERAL = 0,
		DEFAULT,
		INTERNAL,
		MUTABLE_LENGTH,

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

	void UpdateMutableSectionIfNeeded(const String& _name, const DAVA::VariantType& _value);

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