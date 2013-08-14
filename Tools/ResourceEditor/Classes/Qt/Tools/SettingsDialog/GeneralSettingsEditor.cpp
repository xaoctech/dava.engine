/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "GeneralSettingsEditor.h"
#include "SceneEditor/EditorSettings.h"
#include <QHeaderView>

#define SETTINGS_SCREEN_WIDTH		"settingsdialog.screenwidth"
#define SETTINGS_SCREEN_HEIGHT		"settingsdialog.screenheight"
#define SETTINGS_LANGUAGE			"settingsdialog.language"
#define SETTINGS_OUTPUT				"settingsdialog.output"
#define SETTINGS_CAMERA_SPEED_1		"settingsdialog.cameraspeed1"
#define SETTINGS_CAMERA_SPEED_2		"settingsdialog.cameraspeed2"
#define SETTINGS_CAMERA_SPEED_3		"settingsdialog.cameraspeed3"
#define SETTINGS_CAMERA_SPEED_4		"settingsdialog.cameraspeed4"
#define SETTINGS_LEFTPANE_WIDTH		"settingsdialog.leftpanelwidth"
#define SETTINGS_RIGHTPANE_WIDTH	"settingsdialog.rightpanelwidth"
#define SETTINGS_DRAW_GRID			"settingsdialog.drawgrid"
#define SETTINGS_IMPOSTERS			"settingsdialog.imposters"
#define SETTINGS_DESIGNER_NAME		"settingsdialog.designername"
#define SETTINGS_PREVIEW_AT_LIBRARY	"Enable Preview at Library"

#define INITIALIZE_PROPERTY(propertyInstanceName, settingsGetterName, configName)\
	propertyInstanceName = new QtPropertyDataDavaVariant(VariantType(EditorSettings::Instance()->settingsGetterName));\
	AppendProperty(QString((WStringToString(LocalizedString(configName))).c_str()),	propertyInstanceName, NULL);\
	connect(propertyInstanceName,SIGNAL(ValueChanged()),this, SLOT(OnValueChanged()));\
	propertiesSet.push_back(propertyInstanceName);

void GeneralSettingsEditor::InitializeProperties() {
	INITIALIZE_PROPERTY(propertyScreenWidth,	GetScreenWidth(),			SETTINGS_SCREEN_WIDTH)
	INITIALIZE_PROPERTY(propertyScreenHeight,	GetScreenHeight(),			SETTINGS_SCREEN_HEIGHT)
	INITIALIZE_PROPERTY(propertyLanguage,		GetLanguage(),				SETTINGS_LANGUAGE)
	INITIALIZE_PROPERTY(propertyOutput,			GetShowOutput(),			SETTINGS_OUTPUT)
	INITIALIZE_PROPERTY(propertyCameraSpeed1,	GetCameraSpeed(0),			SETTINGS_CAMERA_SPEED_1)
	INITIALIZE_PROPERTY(propertyCameraSpeed2,	GetCameraSpeed(1),			SETTINGS_CAMERA_SPEED_2)
	INITIALIZE_PROPERTY(propertyCameraSpeed3,	GetCameraSpeed(2),			SETTINGS_CAMERA_SPEED_3)
	INITIALIZE_PROPERTY(propertyCameraSpeed4,	GetCameraSpeed(3),			SETTINGS_CAMERA_SPEED_4)
	INITIALIZE_PROPERTY(propertyLeftpaneWidth,	GetLeftPanelWidth(),		SETTINGS_LEFTPANE_WIDTH)
	INITIALIZE_PROPERTY(propertyRightpaneWidth,	GetRightPanelWidth(),		SETTINGS_RIGHTPANE_WIDTH)
	INITIALIZE_PROPERTY(propertyDrawGrid,		GetDrawGrid(),				SETTINGS_DRAW_GRID)
	INITIALIZE_PROPERTY(propertyImposters,		GetEnableImposters(),		SETTINGS_IMPOSTERS)
	INITIALIZE_PROPERTY(propertyDesignerName,	GetDesignerName(),			SETTINGS_DESIGNER_NAME)
	INITIALIZE_PROPERTY(propertyPreviewAtLibrary,GetPreviewDialogEnabled(),	SETTINGS_PREVIEW_AT_LIBRARY)
}

GeneralSettingsEditor::GeneralSettingsEditor( QWidget* parent)
		:QtPropertyEditor(parent)
{
	InitializeProperties();

	propertyLanguage->AddAllowedValue(DAVA::VariantType(String("en")), "en");
	propertyLanguage->AddAllowedValue(DAVA::VariantType(String("ru")), "ru");

	expandAll();

	resizeColumnToContents(0);
}

GeneralSettingsEditor::~GeneralSettingsEditor()
{
	Q_FOREACH(QtPropertyDataDavaVariant * item, propertiesSet)
	{
		delete item;
	}
	propertiesSet.clear();
}

void GeneralSettingsEditor::OnValueChanged()
{
	QtPropertyDataDavaVariant* sender = dynamic_cast<QtPropertyDataDavaVariant*>(QObject::sender());
	if(!sender)
	{
		return;
	}
	
	if(sender == propertyScreenWidth)
	{
		EditorSettings::Instance()->SetScreenWidth(sender->GetValue().toInt());
	}
	else if(sender == propertyScreenHeight)
	{
		EditorSettings::Instance()->SetScreenHeight(sender->GetValue().toInt());
	}
	else if(sender == propertyLanguage)
	{
		EditorSettings::Instance()->SetLanguage(sender->GetValue().toString().toStdString());
	}
	else if(sender == propertyOutput)
	{
		EditorSettings::Instance()->SetShowOuput(sender->GetValue().toBool());
	}
	else if(sender == propertyCameraSpeed1)
	{
		EditorSettings::Instance()->SetCameraSpeed(0, sender->GetValue().toFloat());
	}
	else if(sender == propertyCameraSpeed2)
	{
		EditorSettings::Instance()->SetCameraSpeed(1, sender->GetValue().toFloat());
	}
	else if(sender == propertyCameraSpeed3)
	{
		EditorSettings::Instance()->SetCameraSpeed(2, sender->GetValue().toFloat());
	}
	else if(sender == propertyCameraSpeed4)
	{
		EditorSettings::Instance()->SetCameraSpeed(3, sender->GetValue().toFloat());
	}
	else if(sender == propertyLeftpaneWidth)
	{
		EditorSettings::Instance()->SetLeftPanelWidth(sender->GetValue().toInt());
	}
	else if(sender == propertyRightpaneWidth)
	{
		EditorSettings::Instance()->SetRightPanelWidth(sender->GetValue().toInt());
	}
	else if(sender == propertyDrawGrid)
	{
		EditorSettings::Instance()->SetDrawGrid(sender->GetValue().toBool());
	}
	else if(sender == propertyImposters)
	{
		EditorSettings::Instance()->SetEnableImposters(sender->GetValue().toBool());
	}
	else if(sender == propertyDesignerName)
	{
		EditorSettings::Instance()->SetDesignerName(sender->GetValue().toString().toStdString());
	}
	else if(sender == propertyPreviewAtLibrary)
	{
		EditorSettings::Instance()->SetPreviewDialogEnabled(sender->GetValue().toBool());
	}
	
	EditorSettings::Instance()->Save();
}

