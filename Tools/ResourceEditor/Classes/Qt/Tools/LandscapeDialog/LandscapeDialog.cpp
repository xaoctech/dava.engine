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

#include "LandscapeDialog.h"
#include <QLabel>
#include <QMessageBox>
#include "ui_BaseAddEntityDialog.h"


#define  INIT_PATH_WIDGET(widgetName, widgetNum, widgetTitle, fileFilter) SelectPathWidgetBase* widgetName = new SelectPathWidgetBase(parent,resFolder,"", widgetTitle, fileFilter);\
	if(innerLandscape){\
		widgetName->setText(innerLandscape->GetTextureName(widgetNum).GetAbsolutePathname());\
		widgetMap[widgetName] = DefaultInfo(widgetNum, innerLandscape->GetTextureName(widgetNum));\
	}\
	else{\
		widgetMap[widgetName] = DefaultInfo(widgetNum, FilePath());\
	}\
	widgetName->setEnabled(innerLandscape);\
	connect(widgetName, SIGNAL(PathSelected(DAVA::String)), this, SLOT(ValueChanged(DAVA::String)));

#define OPEN_TEXTURE_TITLE "Open texture"
#define OPEN_HEIGHTMAP_TITLE "Open height map"
#define TEXTURE_FILE_FILTER "PNG(*.png);; TEX(*.tex)"
#define HEIGHTMAP_FILE_FILTER "HeightMap(*.heightmap)"
#define HEIGHT_MAP_ID 0xABCD

#define TAB_CONTENT_WIDTH 450
#define TAB_CONTENT_HEIGHT 380

#define CREATE_TITLE "Create"
#define DELETE_TITLE "Delete"

LandscapeDialog::LandscapeDialog(Entity* _landscapeEntity,  QWidget* parent)
:BaseAddEntityDialog(parent)
{
	setWindowTitle("Landscape Settings");
	setAcceptDrops(false);
	
	delete	ui->propEditor;
	ui->propEditor = new LandscapeSettingsEditor(_landscapeEntity, this);
	ui->propEditor->setMinimumSize(QSize(TAB_CONTENT_WIDTH,TAB_CONTENT_HEIGHT));
	connect(ui->propEditor, SIGNAL(TileModeChanged(int)), this, SLOT(TileModeChanged(int)));
	ui->verticalLayout_4->addWidget(ui->propEditor);

	innerLandscapeEntity = _landscapeEntity;
	defaultLandscapeEntity = _landscapeEntity;

	innerLandscape = DAVA::GetLandscape(innerLandscapeEntity);
	
	DAVA::String resFolder = FilePath("~res:/").GetAbsolutePathname();
	
	INIT_PATH_WIDGET(colorTexutureWidget, Landscape::TEXTURE_COLOR, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture0Widget, Landscape::TEXTURE_TILE0, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture1Widget, Landscape::TEXTURE_TILE1, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture2Widget, Landscape::TEXTURE_TILE2, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture3Widget, Landscape::TEXTURE_TILE3, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileMaskWidget,	  Landscape::TEXTURE_TILE_MASK, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tiledTextureWidget,  Landscape::TEXTURE_TILE_FULL, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	
	SelectPathWidgetBase* heightMapWidget = new SelectPathWidgetBase(parent, resFolder,"",
																	 OPEN_HEIGHTMAP_TITLE, HEIGHTMAP_FILE_FILTER);
	widgetMap[heightMapWidget] = DefaultInfo();
	if(innerLandscape)
	{
		heightMapWidget->setText(innerLandscape->GetHeightmapPathname().GetAbsolutePathname());
		widgetMap[heightMapWidget] = DefaultInfo(HEIGHT_MAP_ID, innerLandscape->GetHeightmapPathname());
	}
	heightMapWidget->setEnabled(innerLandscape);
	connect(heightMapWidget, SIGNAL(PathSelected(DAVA::String)), this, SLOT(ValueChanged(DAVA::String)));
	
	AddControlToUserContainer(colorTexutureWidget, "Color texture:");
	AddControlToUserContainer(tileTexuture0Widget, "Tile texture 0:");
	AddControlToUserContainer(tileTexuture1Widget, "Tile texture 1:");
	AddControlToUserContainer(tileTexuture2Widget, "Tile texture 2:");
	AddControlToUserContainer(tileTexuture3Widget, "Tile texture 3:");
	AddControlToUserContainer(tileMaskWidget, "Tile mask:");
	AddControlToUserContainer(tiledTextureWidget, "Tiled texture:");
	AddControlToUserContainer(heightMapWidget, "Height map:");

	QString btnTitle = CREATE_TITLE;
	if(innerLandscape != NULL)
	{
		TileModeChanged((int)innerLandscape->GetTiledShaderMode());
		btnTitle = DELETE_TITLE;
	}
	actionButton = new QPushButton(btnTitle, this);
	connect(actionButton, SIGNAL(clicked()), this, SLOT(ActionButtonClicked()));
	ui->lowerLayOut->removeWidget(ui->buttonBox);
	ui->lowerLayOut->addWidget(actionButton, 0, 0);
	ui->lowerLayOut->addWidget(ui->buttonBox, 0, 1);
}

LandscapeDialog::~LandscapeDialog()
{
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		delete it->first;
	}
	
	delete actionButton;
}

void LandscapeDialog::ActionButtonClicked()
{
	bool needToCreate = actionButton->text() == CREATE_TITLE;
	if(needToCreate)
	{
		actionButton->setText(DELETE_TITLE);
		
		Entity* entityToProcess = new Entity();
		Landscape* newLandscape = new Landscape();
		newLandscape->SetTiledShaderMode(Landscape::TILED_MODE_TILE_DETAIL_MASK);
		entityToProcess->AddComponent(new RenderComponent(ScopedPtr<Landscape>(newLandscape)));
		entityToProcess->SetName(ResourceEditor::LANDSCAPE_NODE_NAME);
		SetLandscapeEntity(entityToProcess);
	}
	else
	{
		if(innerLandscapeEntity != defaultLandscapeEntity)
		{
			Landscape* newLandscape = DAVA::GetLandscape(innerLandscapeEntity);
			SafeRelease(newLandscape);
			SafeRelease(innerLandscapeEntity);
		}
		
		SetLandscapeEntity(NULL);
		actionButton->setText(CREATE_TITLE);
	}
}

Entity* LandscapeDialog::GetModificatedEntity()
{
	return innerLandscapeEntity;
}


void LandscapeDialog::SetLandscapeEntity(Entity* _landscapeEntity)
{
	innerLandscapeEntity = _landscapeEntity;
	innerLandscape = innerLandscapeEntity== NULL ? NULL : DAVA::GetLandscape(_landscapeEntity);
	LandscapeSettingsEditor* editor = dynamic_cast<LandscapeSettingsEditor*>(ui->propEditor);
	DVASSERT(editor);
	editor->SetLandscapeEntity(innerLandscapeEntity);
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		SelectPathWidgetBase* widget = it->first;
		int32 info = it->second.specificInfo;
		
		if(innerLandscape)
		{
			if(info == HEIGHT_MAP_ID)
			{
				widget->setText(innerLandscape->GetHeightmapPathname().GetAbsolutePathname());
			}
			else
			{
				widget->setText(innerLandscape->GetTextureName((Landscape::eTextureLevel)info).GetAbsolutePathname());
			}
		}
		else
		{
			widget->blockSignals(true);
			widget->setText(DAVA::String(""));
			widget->blockSignals(false);
		}
	
		widget->setEnabled(innerLandscape);
	}
	
	if(innerLandscape != NULL)
	{
		TileModeChanged((int)innerLandscape->GetTiledShaderMode());
	}
	
}

void LandscapeDialog::TileModeChanged(int newValue)
{
	bool shouldEnableControls = (Landscape::eTiledShaderMode )newValue != Landscape::TILED_MODE_TILE_DETAIL_MASK;

	SelectPathWidgetBase* tileTexuture0Widget = FindWidgetBySpecInfo(Landscape::TEXTURE_TILE0);
	SelectPathWidgetBase* tileTexuture1Widget = FindWidgetBySpecInfo(Landscape::TEXTURE_TILE1);
	SelectPathWidgetBase* tileTexuture2Widget = FindWidgetBySpecInfo(Landscape::TEXTURE_TILE2);
	SelectPathWidgetBase* tileTexuture3Widget = FindWidgetBySpecInfo(Landscape::TEXTURE_TILE3);
	
	tileTexuture1Widget->setEnabled(shouldEnableControls);
	tileTexuture2Widget->setEnabled(shouldEnableControls);
	tileTexuture3Widget->setEnabled(shouldEnableControls);
	
	
	
	if(!shouldEnableControls)
	{
		tileTexuture1Widget->blockSignals(true);
		tileTexuture2Widget->blockSignals(true);
		tileTexuture3Widget->blockSignals(true);
		
		DAVA::String path = tileTexuture0Widget->getText();
		tileTexuture1Widget->setText(path);
		tileTexuture2Widget->setText(path);
		tileTexuture3Widget->setText(path);
		
		tileTexuture1Widget->blockSignals(false);
		tileTexuture2Widget->blockSignals(false);
		tileTexuture3Widget->blockSignals(false);
		
	}
	else
	{
		tileTexuture1Widget->setText(widgetMap[tileTexuture1Widget].path.GetAbsolutePathname());
		tileTexuture2Widget->setText(widgetMap[tileTexuture2Widget].path.GetAbsolutePathname());
		tileTexuture3Widget->setText(widgetMap[tileTexuture3Widget].path.GetAbsolutePathname());
	}
}


SelectPathWidgetBase* LandscapeDialog::FindWidgetBySpecInfo(int value)
{
	SelectPathWidgetBase* retValue = NULL;
	for(DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end();++it)
	{
		if(it->second.specificInfo == value)
		{
			retValue = it->first;
			break;
		}
	}
	return retValue;
}

void LandscapeDialog::CleanupPathWidgets()
{
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		it->first->EraseWidget();
	}
}

void LandscapeDialog::ValueChanged(String fileName)
{
	if(!innerLandscape)
	{
		return;
	}
	SelectPathWidgetBase* sender = dynamic_cast<SelectPathWidgetBase*>(QObject::sender());
	
	FilePath filePath(fileName);
	if(widgetMap[sender].specificInfo == HEIGHT_MAP_ID)
	{
		FilePath presentPath = innerLandscape->GetHeightmapPathname();
		if(filePath != presentPath)
		{
			Heightmap* heightmap = new Heightmap();
			heightmap->Load(filePath);
			innerLandscape->SetHeightmap(heightmap);
		}
	}
	else
	{
		int id = widgetMap[sender].specificInfo;
		FilePath presentName = innerLandscape->GetTextureName((Landscape::eTextureLevel)id);
		if(filePath != presentName)
		{
			innerLandscape->SetTexture((Landscape::eTextureLevel)id, filePath);
			if(innerLandscape->GetTiledShaderMode() == Landscape::TILED_MODE_TILE_DETAIL_MASK)
			{
				TileModeChanged(Landscape::TILED_MODE_TILE_DETAIL_MASK);
			}
		}
	}
}

void LandscapeDialog::reject()
{
	if(innerLandscapeEntity != defaultLandscapeEntity)
	{
		SafeRelease(innerLandscapeEntity);
	}
	innerLandscapeEntity = defaultLandscapeEntity;
	innerLandscape = DAVA::GetLandscape(innerLandscapeEntity);
	
	if(!innerLandscape)
	{
		QDialog::reject();
		return;
	}
	
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		String defaultValue(it->second.path.GetAbsolutePathname());
		String present(it->first->getText());
		if(present != defaultValue)
		{
			it->first->setText(defaultValue);
		}
	}

	ui->propEditor->RestoreInitialSettings();
	
	QDialog::reject();
}
