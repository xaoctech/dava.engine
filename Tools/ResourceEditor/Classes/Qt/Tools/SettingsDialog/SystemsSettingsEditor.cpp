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

#include "SystemsSettingsEditor.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"

#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtProperyData/QtPropertyDataDavaVariant.h"
#include "Qt/Settings/SettingsStateDialog.h"
#include "Main/mainwindow.h"
 
#include <QHeaderView>

#define ADD_HEADER(headerName) header = AddHeader(headerName);

#define INIT_PROPERTY(propertyName, getter, rowName, handlerName) QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(getter));\
	AppendProperty(QString(rowName), propertyName, header);\
	connect(propertyName,SIGNAL(ValueChanged()),this, SLOT(handlerName));\
	propertiesMapInitialValues[propertyName] = DAVA::VariantType(getter);

#define INIT_PROPERTY_WITH_BTN(propertyName, propertyNameBtn, getter, rowName, handlerName,drawStateMap) QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(getter));\
	AppendProperty(QString(rowName), propertyName, header);\
	connect(propertyName,SIGNAL(ValueChanged()),this, SLOT(handlerName));\
	propertiesMapInitialValues[propertyName] = DAVA::VariantType(getter);\
	propertyName->SetFlags(QtPropertyData::FLAG_IS_NOT_EDITABLE);\
	QPushButton * propertyNameBtn = CreatePushBnt();\
	propertyName->AddOW(QtPropertyOW(propertyNameBtn));\
	QObject::connect(propertyNameBtn, SIGNAL(pressed()), this, SLOT(ShowDialog()));\
	buttonToFlagsMap[propertyNameBtn]= &drawStateMap;

#define ST_COLL_DRAW_NOTHING_NAME			"Draw nothing"
#define ST_COLL_DRAW_OBJECTS_NAME			"Draw objects"
#define ST_COLL_DRAW_OBJECTS_SELECTED_NAME	"Draw objects selected"
#define ST_COLL_DRAW_OBJECTS_RAYTEST_NAME	"Draw objects raytest"
#define ST_COLL_DRAW_LAND_NAME				"Draw land"
#define ST_COLL_DRAW_LAND_RAYTEST_NAME		"Draw land raytest"
#define ST_COLL_DRAW_LAND_COLLISION_NAME	"Draw land collision"
#define ST_COLL_DRAW_ALL_NAME				"Draw all"

#define ST_MODIF_OFF_NAME	"Off"
#define ST_MODIF_MOVE_NAME	"Move"
#define ST_MODIF_ROTATE_NAME "Rotate"
#define ST_MODIF_SCALE_NAME	"Scale"

#define ST_AXIS_NONE_NAME	"None"
#define ST_AXIS_X_NAME		"X"
#define ST_AXIS_Y_NAME		"Y"
#define ST_AXIS_Z_NAME		"Z"
#define ST_AXIS_XY_NAME		"XY"
#define ST_AXIS_XZ_NAME		"XZ"
#define ST_AXIS_YZ_NAME		"YZ"


#define ST_SELDRAW_NOTHING_NAME			"Nothing"
#define ST_SELDRAW_DRAW_SHAPE_NAME		"Draw share"
#define ST_SELDRAW_DRAW_CORNERS_NAME	"Draw corners"
#define ST_SELDRAW_FILL_SHAPE_NAME		"Fill share"
#define ST_SELDRAW_NO_DEEP_TEST_NAME	"No deep test"
#define ST_SELDRAW_ALL_NAME				"All"

#define ST_PIVOT_ENTITY_CENTER_NAME	"Entity center"
#define ST_PIVOT_COMMON_CENTER_NAME	"Common center"

#define GET_SENDER_CONTENT 	QtPropertyDataDavaVariant* sender = dynamic_cast<QtPropertyDataDavaVariant*>(QObject::sender());\
if(!sender){\
	return;\
}\
VariantType senderContent(sender->GetVariantValue());

SystemsSettingsEditor::SystemsSettingsEditor( QWidget* parent)
		:QtPropertyEditor(parent)
{
	sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
	if (NULL ==sceneEditor)
	{
		return;
	}
	
	InitializeProperties();
	expandAll();
	resizeColumnToContents(0);
}

SystemsSettingsEditor::~SystemsSettingsEditor()
{
	for (DAVA::Map<QtPropertyDataDavaVariant *, DAVA::VariantType >::iterator it= propertiesMapInitialValues.begin(); it != propertiesMapInitialValues.end(); ++it)
	{
		delete it->first;
	}
	//TODO: delete buttons
	propertiesMapInitialValues.clear();
}

void SystemsSettingsEditor::InitializeProperties()
{
	QtPropertyItem *header = NULL;
	
	/*ADD_HEADER("Custom color settings:");
	INIT_PROPERTY(brushSizeProp, sceneEditor->customColorsSystem->GetBrushSize(), "Brush size", HandleCustomColorBrushSize());*/
	/*
	Vector<Color> customColors = EditorConfig::Instance()->GetColorPropertyValues("LandscapeCustomColors");
	Vector<String> customColorsDescription = EditorConfig::Instance()->GetComboPropertyValues("LandscapeCustomColorsDescription");
	
	QtPropertyDataDavaVariant* brushColorProp = new QtPropertyDataDavaVariant(VariantType(customColors[0]));
	AppendProperty(QString("Brush size"), brushColorProp, header);
	connect(brushColorProp,SIGNAL(ValueChanged()),this, SLOT(HandleCustomColorBrushSize()));
	propertiesMap[brushColorProp] = DAVA::VariantType(customColors[0]);

	brushColorProp->AddAllowedValue(DAVA::VariantType(customColors[0]), customColorsDescription[0].c_str());
	brushColorProp->AddAllowedValue(DAVA::VariantType(customColors[1]), customColorsDescription[1].c_str());
	brushColorProp->AddAllowedValue(DAVA::VariantType(customColors[2]), customColorsDescription[2].c_str());*/
	
/*	ADD_HEADER("Camera system settings:");
	INIT_PROPERTY(camSysMoveSpeed, sceneEditor->cameraSystem->GetMoveSpeed(), "Move speed", HandleCameraMoveSpeed());
	Rect rect = sceneEditor->cameraSystem->GetViewportRect();
	INIT_PROPERTY(camSysViewportRect, DAVA::Matrix2(rect.x, rect.y, rect.dx,rect.dy), "Viewport rect", HandleCameraViewportRect());*/
	
	ADD_HEADER("Collision system settings:");
	
	
	collisionSysDrawStateMap[ST_COLL_DRAW_NOTHING_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_NOTHING, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_OBJECTS_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_OBJECTS, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_OBJECTS_SELECTED_NAME]= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_OBJECTS_SELECTED, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_OBJECTS_RAYTEST_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_OBJECTS_RAYTEST, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_LAND_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_LAND, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_LAND_RAYTEST_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_LAND_RAYTEST, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_LAND_COLLISION_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_LAND_COLLISION, false);
	collisionSysDrawStateMap[ST_COLL_DRAW_ALL_NAME]				= std::make_pair<DAVA::uint32,bool>(ST_COLL_DRAW_ALL, false);

	InitMapWithFlag(collisionSysDrawStateMap, sceneEditor->collisionSystem->GetDebugDrawFlags());
	INIT_PROPERTY_WITH_BTN(collSysDrawMode, collSysDrawModeBtn,ResolveMapToString(collisionSysDrawStateMap), "Collision draw mode", HandleCollisionDrawMode(),collisionSysDrawStateMap);
	/*collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_NOTHING), ST_COLL_DRAW_NOTHING_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_OBJECTS), ST_COLL_DRAW_OBJECTS_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_OBJECTS_SELECTED), ST_COLL_DRAW_OBJECTS_SELECTED_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_OBJECTS_RAYTEST), ST_COLL_DRAW_OBJECTS_RAYTEST_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_LAND), ST_COLL_DRAW_LAND_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_LAND_RAYTEST), ST_COLL_DRAW_LAND_RAYTEST_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_LAND_COLLISION), ST_COLL_DRAW_LAND_COLLISION_NAME);
	collSysDrawMode->AddAllowedValue(DAVA::VariantType(ST_COLL_DRAW_ALL), ST_COLL_DRAW_ALL_NAME);*/
	
/*	ADD_HEADER("Hood system settings:");
	INIT_PROPERTY(hoodSysModifMode, sceneEditor->hoodSystem->GetModifMode(), "Modif. mode", HandleHoodModifMode());
	hoodSysModifMode->AddAllowedValue(DAVA::VariantType(ST_MODIF_OFF), ST_MODIF_OFF_NAME);
	hoodSysModifMode->AddAllowedValue(DAVA::VariantType(ST_MODIF_MOVE), ST_MODIF_MOVE_NAME);
	hoodSysModifMode->AddAllowedValue(DAVA::VariantType(ST_MODIF_ROTATE), ST_MODIF_ROTATE_NAME);
	hoodSysModifMode->AddAllowedValue(DAVA::VariantType(ST_MODIF_SCALE), ST_MODIF_SCALE_NAME);
	INIT_PROPERTY(hoodSysPosition, sceneEditor->hoodSystem->GetPosition(), "Position", HandleHoodPosition());
	INIT_PROPERTY(hoodModifAxis, sceneEditor->hoodSystem->GetModifAxis(), "Modif. axis", HandleHoodModifAxis());
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_NONE), ST_AXIS_NONE_NAME);
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_X), ST_AXIS_X_NAME);
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_Y), ST_AXIS_Y_NAME);
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_Z), ST_AXIS_Z_NAME);
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_XY), ST_AXIS_XY_NAME);
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_XZ), ST_AXIS_XZ_NAME);
	hoodModifAxis->AddAllowedValue(DAVA::VariantType(ST_AXIS_YZ), ST_AXIS_YZ_NAME);
	INIT_PROPERTY(hoodScale, sceneEditor->hoodSystem->GetScale(), "Scale", HandleHoodScale());
	
	ADD_HEADER("Selection system settings:");

	selectionSysDrawStateMap[ST_SELDRAW_NOTHING_NAME]		= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_NOTHING, false);
	selectionSysDrawStateMap[ST_SELDRAW_DRAW_SHAPE_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_DRAW_SHAPE, false);
	selectionSysDrawStateMap[ST_SELDRAW_DRAW_CORNERS_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_DRAW_CORNERS, false);
	selectionSysDrawStateMap[ST_SELDRAW_FILL_SHAPE_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_FILL_SHAPE, false);
	selectionSysDrawStateMap[ST_SELDRAW_NO_DEEP_TEST_NAME]	= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_NO_DEEP_TEST, false);
	selectionSysDrawStateMap[ST_SELDRAW_ALL_NAME]			= std::make_pair<DAVA::uint32,bool>(ST_SELDRAW_ALL, false);
	
	DAVA::String valueDescrition = ResolveFlags(selectionSysDrawStateMap,sceneEditor->selectionSystem->GetDrawMode());
	INIT_PROPERTY_WITH_BTN(selectionDrawMode, selectionDrawModeBtn,valueDescrition, "Draw mode", HandleSelectionDrawMode(),selectionSysDrawStateMap);*/
	/*selectionDrawMode->SetFlags(QtPropertyData::FLAG_IS_NOT_EDITABLE);
	
	QPushButton *configBtn = CreatePushBnt();
	selectionDrawMode->AddOW(QtPropertyOW(configBtn));
	QObject::connect(configBtn, SIGNAL(pressed()), this, SLOT(ShowDialog()));
	buttonToFlagsMap[configBtn]= &selectionSysDrawStateMap;*/
	
	/*INIT_PROPERTY(selectionPivotPoint, sceneEditor->selectionSystem->GetPivotPoint(), "Pivot point", HandlePivotPoint());
	selectionPivotPoint->AddAllowedValue(DAVA::VariantType(ST_PIVOT_ENTITY_CENTER), ST_PIVOT_ENTITY_CENTER_NAME);
	selectionPivotPoint->AddAllowedValue(DAVA::VariantType(ST_PIVOT_COMMON_CENTER), ST_PIVOT_COMMON_CENTER_NAME);*/

	
}

void SystemsSettingsEditor::RestoreInitialSettings()
{
	for (DAVA::Map<QtPropertyDataDavaVariant *, DAVA::VariantType>::iterator it= propertiesMapInitialValues.begin(); it != propertiesMapInitialValues.end(); ++it)
	{
		it->first->SetVariantValue(it->second);
	}
}

void SystemsSettingsEditor::InitMapWithFlag(STATE_FLAGS_MAP& map, DAVA::uint32 value)
{
	for(STATE_FLAGS_MAP::iterator it = map.begin(); it !=  map.end(); ++it)
	{
		DAVA::uint32 flag = it->second.first;
		
		if(value==0 && flag == 0)
		{
			it->second.second = true;
			break;
		}
		
		if(flag == std::numeric_limits<DAVA::uint32>::max() )//FF...
		{
			if(flag == value)
			{
				it->second.second = true;
				break;
			}
			continue;
		}
		
		if(flag & value)
		{
			it->second.second = true;
		}
	}
}

DAVA::String SystemsSettingsEditor::ResolveMapToString(STATE_FLAGS_MAP& map)
{
	DAVA::String retValue = "";
	
	for(DAVA::Map<DAVA::String,std::pair<DAVA::uint32, bool> >::iterator it = map.begin(); it !=  map.end(); ++it)
	{
		DAVA::String description = it->first;
		bool isChecked = it->second.second;
		
		if(isChecked)
		{
			retValue += ( " " + description);
		}
	}
	return retValue;
}

DAVA::uint32 SystemsSettingsEditor::ResolveMapToUint(STATE_FLAGS_MAP& map)
{
	DAVA::uint32 retValue = 0;
	
	for(STATE_FLAGS_MAP::iterator it = map.begin(); it !=  map.end(); ++it)
	{
		bool isChecked = it->second.second;
		DAVA::uint32 flag = it->second.first;
		
		if(isChecked)
		{
			retValue |= flag;
		}
	}
	return retValue;
}

void SystemsSettingsEditor::ShowDialog()
{
	QPushButton * sender = dynamic_cast<QPushButton *>(QObject::sender());
	DVASSERT(sender);
	STATE_FLAGS_MAP* propFlagsMap = buttonToFlagsMap[sender];
	
	SettingsStateDialog dialog(propFlagsMap,this);
	dialog.exec();
	
	if(dialog.result() != QDialog::Accepted )
	{
		return;
	}
	QObject* viewPort = sender->parent();
	QtPropertyDataDavaVariant* propertyToChange = NULL;
	
	for(PROPERTY_INITIAL_VAL_MAP::iterator it = propertiesMapInitialValues.begin(); it !=  propertiesMapInitialValues.end(); ++it)
	{
		if (it->first->GetOWViewport() ==viewPort)
		{
			propertyToChange = it->first;
			break;
		}
	}
	
	DVASSERT(propertyToChange);
	propertyToChange->SetValue(QVariant(ResolveMapToString(*propFlagsMap).c_str()));
}

QPushButton * SystemsSettingsEditor::CreatePushBnt()
{
	QPushButton *configBtn = new QPushButton(QIcon(":/QtIcons/settings.png"), "");
	configBtn->setIconSize(QSize(12, 12));
	configBtn->setFlat(true);
	buttonsList.push_back(configBtn);
	return configBtn;
}

void SystemsSettingsEditor::HandleCustomColorBrushSize()
{
	GET_SENDER_CONTENT
	int32 max = 130;//TODO: get from custom properties
	int32 min = 0;// TODO: ...
	int32 current = senderContent.AsInt32();
	if(current > max || current < min)
	{
		sender->SetVariantValue(DAVA::VariantType(max));
		return;
	}
	if(current < min)
	{
		sender->SetVariantValue(DAVA::VariantType(min));
		return;
	}
	
	sceneEditor->customColorsSystem->SetBrushSize(senderContent.AsInt32(), false);
}

void SystemsSettingsEditor::HandleCameraMoveSpeed()
{
	GET_SENDER_CONTENT	
	float speed = senderContent.AsFloat();
	sceneEditor->cameraSystem->SetMoveSpeed(speed);
}

void SystemsSettingsEditor::HandleCameraViewportRect()
{
	GET_SENDER_CONTENT
	Matrix2 rect = senderContent.AsMatrix2();
	sceneEditor->cameraSystem->SetViewportRect(Rect(rect._00, rect._01, rect._10, rect._11));
}

void SystemsSettingsEditor::HandleCollisionDrawMode()
{
	GET_SENDER_CONTENT	
	DAVA::uint32 value = ResolveMapToUint(collisionSysDrawStateMap);
	sceneEditor->collisionSystem->SetDrawMode(value);
}

void SystemsSettingsEditor::HandleHoodModifMode()
{
	GET_SENDER_CONTENT
	int32 value = senderContent.AsInt32();
	sceneEditor->hoodSystem->SetModifMode((ST_ModifMode)value);
}

void SystemsSettingsEditor::HandleHoodPosition()
{
	GET_SENDER_CONTENT
	Vector3 value = senderContent.AsVector3();
	sceneEditor->hoodSystem->SetPosition(value);
}

void SystemsSettingsEditor::HandleHoodModifAxis()
{
	GET_SENDER_CONTENT
	uint32 value = senderContent.AsUInt32();
	sceneEditor->hoodSystem->SetModifAxis((ST_Axis) value);
}

void SystemsSettingsEditor::HandleHoodScale()
{
	GET_SENDER_CONTENT
	float value = senderContent.AsFloat();
	sceneEditor->hoodSystem->SetScale(value);
}

void SystemsSettingsEditor::HandleSelectionDrawMode()
{
	GET_SENDER_CONTENT
	uint32 value = senderContent.AsUInt32();
	sceneEditor->selectionSystem->SetDrawMode(value);
}

void SystemsSettingsEditor::HandlePivotPoint()
{
	GET_SENDER_CONTENT
	uint32 value = senderContent.AsUInt32();
	sceneEditor->selectionSystem->SetPivotPoint((ST_PivotPoint)value);
}

