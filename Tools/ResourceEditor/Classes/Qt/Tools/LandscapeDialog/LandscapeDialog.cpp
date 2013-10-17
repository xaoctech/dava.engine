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
#include "SceneEditor/EditorSettings.h"
#include "Main/mainwindow.h"
#include "Classes/Commands2/EntityAddCommand.h"
#include "Classes/Commands2/EntityRemoveCommand.h"

#define  INIT_PATH_WIDGET(widgetName, widgetNum, widgetTitle, fileFilter) SelectPathWidgetBase* widgetName = new SelectPathWidgetBase(parent,resFolder,"", widgetTitle, fileFilter);\
	if(innerLandscape){\
		widgetName->setText(innerLandscape->GetTextureName(widgetNum).GetAbsolutePathname());\
		widgetMap[widgetName] = DefaultInfo(widgetNum, innerLandscape->GetTextureName(widgetNum));\
	}\
	else{\
		widgetMap[widgetName] = DefaultInfo(widgetNum, FilePath());\
	}\
	widgetName->setEnabled(innerLandscape);\
	widgetName->SetAllowedFormatsList(textureFormats);\
	connect(widgetName, SIGNAL(PathSelected(DAVA::String)), this, SLOT(ValueChanged(DAVA::String)));

#define OPEN_TEXTURE_TITLE "Open texture"
#define OPEN_HEIGHTMAP_TITLE "Open height map"
#define TEXTURE_FILE_FILTER "PNG(*.png);; TEX(*.tex)"
#define HEIGHTMAP_FILE_FILTER "HeightMap(*.heightmap)"
#define HEIGHT_MAP_ID 0xABCD

#define TAB_CONTENT_WIDTH 450
#define TAB_CONTENT_HEIGHT 300

#define CREATE_TITLE "Create"
#define DELETE_TITLE "Delete"

LandscapeDialog::LandscapeDialog(Entity* _landscapeEntity,  QWidget* parent)
:BaseAddEntityDialog(parent)
{
	setWindowTitle("Landscape Settings");
	setAcceptDrops(true);
	setAttribute( Qt::WA_DeleteOnClose, true );
	DAVA::List<DAVA::String> textureFormats;
	textureFormats.push_back(".tex");
	textureFormats.push_back(".png");
	
	DAVA::List<DAVA::String> heightMapFormats;
	heightMapFormats.push_back(".heightmap");

	entity = SafeRetain(_landscapeEntity);// release in BaseAddEntityDialog::~BaseAddEntityDialog()
	
	innerLandscapeEntity = entity;
	innerLandscape = DAVA::GetLandscape(innerLandscapeEntity);
	
	DAVA::String resFolder = EditorSettings::Instance()->GetDataSourcePath().GetAbsolutePathname();
	
	INIT_PATH_WIDGET(colorTexutureWidget, Landscape::TEXTURE_COLOR, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture0Widget, Landscape::TEXTURE_TILE0, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture1Widget, Landscape::TEXTURE_TILE1, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture2Widget, Landscape::TEXTURE_TILE2, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture3Widget, Landscape::TEXTURE_TILE3, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileMaskWidget,	  Landscape::TEXTURE_TILE_MASK, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tiledTextureWidget,  Landscape::TEXTURE_TILE_FULL, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	
	SelectPathWidgetBase* heightMapWidget = new SelectPathWidgetBase(parent, resFolder,"",
																	 OPEN_HEIGHTMAP_TITLE, HEIGHTMAP_FILE_FILTER);
	widgetMap[heightMapWidget] = DefaultInfo(HEIGHT_MAP_ID, FilePath());
	if(innerLandscape)
	{
		heightMapWidget->setText(innerLandscape->GetHeightmapPathname().GetAbsolutePathname());
		widgetMap[heightMapWidget] = DefaultInfo(HEIGHT_MAP_ID, innerLandscape->GetHeightmapPathname());
	}
	heightMapWidget->setEnabled(innerLandscape);
	heightMapWidget->SetAllowedFormatsList(heightMapFormats);
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

void LandscapeDialog::showEvent ( QShowEvent * event )
{
	BaseAddEntityDialog::showEvent(event);
	SetLandscapeEntity(innerLandscapeEntity);
}

void LandscapeDialog::InitPropertyEditor()
{
	propEditor = new LandscapeSettingsEditor(this);
	connect(propEditor, SIGNAL(TileModeChanged(int)), this, SLOT(TileModeChanged(int)));
	ui->verticalLayout_4->addWidget(propEditor);
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
		RenderComponent* component = new RenderComponent(ScopedPtr<Landscape>(newLandscape));
		entityToProcess->AddComponent(component);
		SafeRelease(component);
		entityToProcess->SetName(ResourceEditor::LANDSCAPE_NODE_NAME);
		SetLandscapeEntity(entityToProcess);
	}
	else
	{
		actionButton->setText(CREATE_TITLE);
		if(entity != innerLandscapeEntity)
		{
			// release should be performed only for created in if(needToCreate){...} entity
			innerLandscapeEntity->RemoveComponent(Component::RENDER_COMPONENT);
			SafeRelease(innerLandscapeEntity);
		}

		SetLandscapeEntity(NULL);
	}
}

void LandscapeDialog::SetLandscapeEntity(Entity* _landscapeEntity)
{
	innerLandscapeEntity = _landscapeEntity;
	innerLandscape = innerLandscapeEntity == NULL ? NULL : DAVA::GetLandscape(_landscapeEntity);
	FillUIbyLandscapeEntity(innerLandscapeEntity);
}


void LandscapeDialog::FillUIbyLandscapeEntity(Entity* _landscapeEntity)
{
	Landscape*	landscapeToProcess =  _landscapeEntity== NULL ? NULL : DAVA::GetLandscape(_landscapeEntity);
	
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		SelectPathWidgetBase* widget = it->first;
		int32 info = it->second.specificInfo;
		
		if(landscapeToProcess)
		{
			if(info == HEIGHT_MAP_ID)
			{
				widget->setText(landscapeToProcess->GetHeightmapPathname().GetAbsolutePathname());
			}
			else
			{
				widget->setText(landscapeToProcess->GetTextureName((Landscape::eTextureLevel)info).GetAbsolutePathname());
			}
		}
		else
		{
			widget->blockSignals(true);
			widget->setText(DAVA::String(""));
			widget->blockSignals(false);
		}
		
		widget->setEnabled(landscapeToProcess);
	}
	
	if(landscapeToProcess != NULL)
	{
		TileModeChanged((int)landscapeToProcess->GetTiledShaderMode());
	}
	propEditor->SetNode(_landscapeEntity);
	propEditor->expandAll();
	PerformResize();
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
			innerLandscape->BuildLandscapeFromHeightmapImage(filePath, innerLandscape->GetBoundingBox());
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
	if(innerLandscapeEntity != entity)
	{
		innerLandscapeEntity->RemoveComponent(Component::RENDER_COMPONENT);
		SafeRelease(innerLandscapeEntity);//release just created entity
	}
	
	innerLandscape = DAVA::GetLandscape(entity);
	
	if(entity)
	{
		for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
		{
			String defaultValue(it->second.path.GetAbsolutePathname());
			String present(it->first->getText());
			if(present != defaultValue)
			{
				it->first->setText(defaultValue);
			}
		}
		
		propEditor->RestoreInitialSettings();
	}
	QDialog::reject();
}


void LandscapeDialog::accept()
{
	SceneEditor2* sceneEditor = QtMainWindow::Instance()->GetCurrentScene();
	
	if(entity == NULL && innerLandscapeEntity != NULL)
	{
		EntityAddCommand* command = new EntityAddCommand(innerLandscapeEntity, sceneEditor);
		sceneEditor->Exec(command);
		sceneEditor->selectionSystem->SetSelection(innerLandscapeEntity);
		SafeRelease(innerLandscapeEntity);
	}
	if(entity != NULL && innerLandscapeEntity != NULL  && (entity != innerLandscapeEntity))
	{
		EntityRemoveCommand * command = new EntityRemoveCommand(entity);
		sceneEditor->Exec(command);
		EntityAddCommand* commandAdd = new EntityAddCommand(innerLandscapeEntity, sceneEditor);
		sceneEditor->Exec(commandAdd);
		sceneEditor->selectionSystem->SetSelection(innerLandscapeEntity);
		SafeRelease(innerLandscapeEntity);
	}
	
	if(entity != NULL && innerLandscapeEntity == NULL)
	{
		EntityRemoveCommand * command = new EntityRemoveCommand(entity);
		sceneEditor->Exec(command);
	}
	
	QDialog::accept();
}
