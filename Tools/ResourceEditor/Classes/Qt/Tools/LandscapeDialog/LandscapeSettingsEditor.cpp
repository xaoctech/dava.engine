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



#include "LandscapeSettingsEditor.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"

#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "Qt/Settings/SettingsStateDialog.h"
#include "Qt/Main/QtUtils.h"
#include "Main/mainwindow.h"

#include <QHeaderView>

#define ADD_HEADER(headerName) header = AddHeader(headerName);

#define INIT_PROPERTY(propertyName, getter, rowName, handlerName) QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(getter));\
	AppendProperty(QString(rowName), propertyName, header);\
	connect(propertyName,SIGNAL(ValueChanged()),this, SLOT(handlerName));\
	propertiesMap.push_back(PropertyInfo(propertyName, QVariant(getter)));

#define INIT_COLOR_PROPERTY(propertyName, getter, rowName, handlerName) \
	QtPropertyDataDavaVariant* propertyName = new QtPropertyDataDavaVariant(VariantType(getter));\
	AppendProperty(QString(rowName), propertyName, header);\
	connect(propertyName,SIGNAL(ValueChanged()),this, SLOT(handlerName));\
	propertiesMap.push_back(PropertyInfo(propertyName, QVariant(ColorToQColor(getter))));

#define GET_SENDER_CONTENT 	QtPropertyDataDavaVariant* sender = dynamic_cast<QtPropertyDataDavaVariant*>(QObject::sender());\
if(!sender){\
	return;\
}\
VariantType senderContent(sender->GetVariantValue());

#define DEFAULT_LANDSCAPE_SIDE_LENGTH	445.0f
#define DEFAULT_LANDSCAPE_HEIGHT		50.0f

LandscapeSettingsEditor::LandscapeSettingsEditor(Entity* landscapeEntity, QWidget* parent)
		:PropertyEditorDialog(parent),
		landscapeEntity(landscapeEntity)
{
	tiledModes.push_back("Tile mask mode");
	tiledModes.push_back("Texture mode");
	tiledModes.push_back("Mixed mode");
	tiledModes.push_back("Detail mask mode");

	SetLandscapeEntity(landscapeEntity);
}

LandscapeSettingsEditor::~LandscapeSettingsEditor()
{
	RemovePropertyAll();
	propertiesMap.clear();
}

void LandscapeSettingsEditor::SetLandscapeEntity(Entity* _landscapeEntity)
{
	RemovePropertyAll();
	propertiesMap.clear();
		
	landscapeEntity = _landscapeEntity;
	if(landscapeEntity == NULL)
	{
		return;
	}
	
	InitializeProperties(landscapeEntity);
	expandAll();
	resizeColumnToContents(0);
}

void LandscapeSettingsEditor::InitializeProperties(Entity* landscapeEntity)
{
	propertyList = landscapeEntity->GetCustomProperties();
	landscape = DAVA::GetLandscape(landscapeEntity);
	
	QtPropertyItem *header = NULL;
		
	ADD_HEADER("Used in static lightning:");
	INIT_PROPERTY(usedInLightning, propertyList->GetBool("editor.staticlight.enable", false), "Used in static lightning", HandleStaticLightEnabled());
	INIT_PROPERTY(castShadows, propertyList->GetBool("editor.staticlight.castshadows", false), "Cast shadows", HandleCastShadows());
	INIT_PROPERTY(receiveShadows, propertyList->GetBool("editor.staticlight.receiveshadows", false), "Receive shadows:", HandleReceiveShadows());

	ADD_HEADER("Fog settings:");
	INIT_PROPERTY(fogEnabled, landscape->IsFogEnabled(), "Fog enabled:", HandleFogEnabled());
	INIT_PROPERTY(fogDencity, landscape->GetFogDensity(), "Dencity:", HandleFogDensity());
	INIT_COLOR_PROPERTY(fogColor, landscape->GetFogColor(), "Fog color:", HandleFogColor());
		
	ADD_HEADER("Landscape:");
	size = DAVA::Vector3(DEFAULT_LANDSCAPE_SIDE_LENGTH, DEFAULT_LANDSCAPE_SIDE_LENGTH, DEFAULT_LANDSCAPE_HEIGHT);
	DAVA::AABBox3 bbox = landscape->GetBoundingBox();
	DAVA::AABBox3 emptyBox;
	if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
	{
		DAVA::AABBox3 transformedBox = bbox;
		if(NULL != landscape->GetWorldTransformPtr())
		{
			bbox.GetTransformedBox(*landscape->GetWorldTransformPtr(), transformedBox);
		}
		size = transformedBox.max - transformedBox.min;
	}
	
	INIT_PROPERTY(sizeLandscape, size.x, "Size:", HandleLandSize());
	INIT_PROPERTY(heightLandscape, size.z, "Height:", HandleLandHeight());
	
	String sMode =  tiledModes[landscape->GetTiledShaderMode()];
	QtPropertyDataDavaVariant* tiledMode = new QtPropertyDataDavaVariant(VariantType(sMode));
	AppendProperty(QString("Tile mode:"), tiledMode, header);
	connect(tiledMode,SIGNAL(ValueChanged()),this, SLOT(HandleTileMode()));
	propertiesMap.push_back(PropertyInfo(tiledMode, QVariant(sMode.c_str())));
	tiledMode->AddAllowedValue(DAVA::VariantType(sMode), sMode.c_str());
	if(landscape->GetTiledShaderMode() != Landscape::TILED_MODE_TILE_DETAIL_MASK)
	{
		tiledMode->AddAllowedValue(DAVA::VariantType(String("Detail mask mode")), "Detail mask mode");
	}

	INIT_COLOR_PROPERTY(tilecolor0, landscape->GetTileColor(Landscape::TEXTURE_TILE0), "Tile color 0:", HandleTileColor0());
	INIT_COLOR_PROPERTY(tilecolor1, landscape->GetTileColor(Landscape::TEXTURE_TILE1), "Tile color 1:", HandleTileColor1());
	INIT_COLOR_PROPERTY(tilecolor2, landscape->GetTileColor(Landscape::TEXTURE_TILE2), "Tile color 2:", HandleTileColor2());
	INIT_COLOR_PROPERTY(tilecolor3, landscape->GetTileColor(Landscape::TEXTURE_TILE3), "Tile color 3:", HandleTileColor3());
}

void LandscapeSettingsEditor::RestoreInitialSettings()
{
	for (DAVA::List<PropertyInfo>::iterator it= propertiesMap.begin(); it != propertiesMap.end(); ++it)
	{
		QtPropertyDataDavaVariant* property = it->property;
		this->model()->blockSignals(true);
		property->SetValue(it->defaultValue);
		this->model()->blockSignals(false);
	}
}

void LandscapeSettingsEditor::HandleStaticLightEnabled()
{
	GET_SENDER_CONTENT;
	propertyList->SetBool("editor.staticlight.enable", senderContent.AsBool());
}

void LandscapeSettingsEditor::HandleCastShadows()
{
	GET_SENDER_CONTENT;
	propertyList->SetBool("editor.staticlight.castshadows", senderContent.AsBool());
}

void LandscapeSettingsEditor::HandleReceiveShadows()
{
	GET_SENDER_CONTENT;
	propertyList->SetBool("editor.staticlight.receiveshadows", senderContent.AsBool());
}

void LandscapeSettingsEditor::HandleFogEnabled()
{
	GET_SENDER_CONTENT;
	landscape->SetFog( senderContent.AsBool());
}

void LandscapeSettingsEditor::HandleFogDensity()
{
	GET_SENDER_CONTENT;
	landscape->SetFogDensity(senderContent.AsFloat());
}

void LandscapeSettingsEditor::HandleLandSize()
{
	GET_SENDER_CONTENT;
	float newSize = senderContent.AsFloat();
	size.x = newSize;
	size.y = newSize;
	AppleNewLandscapeSize();
}

void LandscapeSettingsEditor::AppleNewLandscapeSize()
{
	AABBox3 bbox;
	bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
	bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
	
	FilePath heightMap = landscape->GetHeightmapPathname();
	landscape->BuildLandscapeFromHeightmapImage(heightMap, bbox);
	
	// let transform system recalc worldtranform for landscape entity
	if(NULL != landscapeEntity->GetScene())
	{
		landscapeEntity->GetScene()->transformSystem->Process();
	}
}

void LandscapeSettingsEditor::HandleLandHeight()
{
	GET_SENDER_CONTENT;
	float newHeight = senderContent.AsFloat();
	size.z = newHeight;
	AppleNewLandscapeSize();
}

void LandscapeSettingsEditor::HandleFogColor()
{
	GET_SENDER_CONTENT;
	DAVA::Color color = senderContent.AsColor();
	landscape->SetFogColor(color);
}

void LandscapeSettingsEditor::HandleTileColor0()
{
	GET_SENDER_CONTENT;
	DAVA::Color color = senderContent.AsColor();
	landscape->SetTileColor(Landscape::TEXTURE_TILE0, color);
}

void LandscapeSettingsEditor::HandleTileColor1()
{
	GET_SENDER_CONTENT;
	DAVA::Color color = senderContent.AsColor();
	landscape->SetTileColor(Landscape::TEXTURE_TILE1, color);
}

void LandscapeSettingsEditor::HandleTileColor2()
{
	GET_SENDER_CONTENT;
	DAVA::Color color = senderContent.AsColor();
	landscape->SetTileColor(Landscape::TEXTURE_TILE2, color);
}

void LandscapeSettingsEditor::HandleTileColor3()
{
	GET_SENDER_CONTENT;
	DAVA::Color color = senderContent.AsColor();
	landscape->SetTileColor(Landscape::TEXTURE_TILE3, color);
}

void LandscapeSettingsEditor::HandleTileMode()
{
	GET_SENDER_CONTENT;
	DAVA::String value = senderContent.AsString();
	Landscape::eTiledShaderMode selectedMode=landscape->GetTiledShaderMode();
	for(int32 i = 0; i < tiledModes.size(); ++i)
	{
		if(value == tiledModes[i])
		{
			selectedMode = (Landscape::eTiledShaderMode)i;
			break;
		}
	}
	landscape->SetTiledShaderMode(selectedMode);
	emit TileModeChanged((int)selectedMode);
}
