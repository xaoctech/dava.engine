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

#define  INIT_PATH_WIDGET(widgetName, widgetNum, widgetTitle, fileFilter) SelectPathWidgetBase* widgetName = new SelectPathWidgetBase(parent,resFolder,"", widgetTitle, fileFilter);\
widgetMap[widgetName] = DefaultInfo(widgetNum, landscape->GetTextureName(widgetNum));\
widgetName->setText(landscape->GetTextureName(widgetNum).GetAbsolutePathname());\
connect(widgetName, SIGNAL(PathSelected(DAVA::String)), this, SLOT(ValueChanged(DAVA::String)));

#define OPEN_TEXTURE_TITLE "Open texture"
#define TEXTURE_FILE_FILTER "PNG(*.png);; TEX(*.tex)"
#define HEIGHT_MAP_ID 0xABCD

#define TAB_CONTENT_WIDTH 450
#define TAB_CONTENT_HEIGHT 380

LandscapeDialog::LandscapeDialog(Entity* landscapeEntity,  QWidget* parent)
:QDialog(parent)
{
	setWindowTitle("Landscape Settings");
	
	editor = new LandscapeSettingsEditor(landscapeEntity, this);
	editor->setMinimumSize(QSize(TAB_CONTENT_WIDTH,TAB_CONTENT_HEIGHT));
	connect(editor, SIGNAL(TileModeChanged(int)), this, SLOT(TileModeChanged(int)));
	
	btnBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);;
	connect(btnBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(btnBox, SIGNAL(rejected()), this, SLOT(reject()));
	
	mainLayout = new QVBoxLayout;
	mainLayout->addWidget(editor);
		
	setAcceptDrops(false);
	landscape = DAVA::GetLandscape(landscapeEntity);
	DVASSERT(landscape);
	
	DAVA::String resFolder = FilePath("~res:").GetAbsolutePathname();
	
	INIT_PATH_WIDGET(colorTexutureWidget, Landscape::TEXTURE_COLOR, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture0Widget, Landscape::TEXTURE_TILE0, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture1Widget, Landscape::TEXTURE_TILE1, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture2Widget, Landscape::TEXTURE_TILE2, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileTexuture3Widget, Landscape::TEXTURE_TILE3, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tileMaskWidget,	  Landscape::TEXTURE_TILE_MASK, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	INIT_PATH_WIDGET(tiledTextureWidget,  Landscape::TEXTURE_TILE_FULL, OPEN_TEXTURE_TITLE, TEXTURE_FILE_FILTER);
	
	SelectPathWidgetBase* heightMapWidget = new SelectPathWidgetBase(parent,resFolder,"", "Open height map", "HeightMap(*.heightmap)");
	widgetMap[heightMapWidget] = DefaultInfo(HEIGHT_MAP_ID, landscape->GetHeightmapPathname());
	heightMapWidget->setText(landscape->GetHeightmapPathname().GetAbsolutePathname());
	connect(heightMapWidget, SIGNAL(PathSelected(DAVA::String)), this, SLOT(ValueChanged(DAVA::String)));
	
	QLabel* label1 = new QLabel("Color texture:", parent);
	QLabel* label2 = new QLabel("Tile texture 0:", parent);
	QLabel* label3 = new QLabel("Tile texture 1:", parent);
	QLabel* label4 = new QLabel("Tile texture 2:", parent);
	QLabel* label5 = new QLabel("Tile texture 3:", parent);
	QLabel* label6 = new QLabel("Tile mask:", parent);
	QLabel* label7 = new QLabel("Tiled texture:", parent);
	QLabel* label8 = new QLabel("Height map:", parent);
	
	
	mainLayout->addWidget(label1);
	mainLayout->addWidget(colorTexutureWidget);
	mainLayout->addWidget(label2);
	mainLayout->addWidget(tileTexuture0Widget);
	mainLayout->addWidget(label3);
	mainLayout->addWidget(tileTexuture1Widget);
	mainLayout->addWidget(label4);
	mainLayout->addWidget(tileTexuture2Widget);
	mainLayout->addWidget(label5);
	mainLayout->addWidget(tileTexuture3Widget);
	mainLayout->addWidget(label6);
	mainLayout->addWidget(tileMaskWidget);
	mainLayout->addWidget(label7);
	mainLayout->addWidget(tiledTextureWidget);
	mainLayout->addWidget(label8);
	mainLayout->addWidget(heightMapWidget);
	
	additionalWidgets.push_back(label1);
	additionalWidgets.push_back(label2);
	additionalWidgets.push_back(label3);
	additionalWidgets.push_back(label4);
	additionalWidgets.push_back(label5);
	additionalWidgets.push_back(label6);
	additionalWidgets.push_back(label7);
	additionalWidgets.push_back(label8);
	
	mainLayout->addWidget(btnBox);
	setLayout(mainLayout);
	
	DAVA::Landscape * landscape = DAVA::GetLandscape(landscapeEntity);
	TileModeChanged((int)landscape->GetTiledShaderMode());
}

LandscapeDialog::~LandscapeDialog()
{
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		delete it->first;
	}
	
	Q_FOREACH(QWidget* widget, additionalWidgets)
	{
		delete widget;
	}
	
	delete editor;
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
		DAVA::String path = tileTexuture0Widget->getText();
		tileTexuture1Widget->setText(path);
		tileTexuture2Widget->setText(path);
		tileTexuture3Widget->setText(path);
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
	SelectPathWidgetBase* sender = dynamic_cast<SelectPathWidgetBase*>(QObject::sender());
	FilePath filePath(fileName);
	if(widgetMap[sender].specificInfo == HEIGHT_MAP_ID)
	{
		Heightmap* heightmap = new Heightmap();
		heightmap->Load(filePath);
		landscape->SetHeightmap(heightmap);
	}
	else
	{
		landscape->SetTexture((Landscape::eTextureLevel)widgetMap[sender].specificInfo, filePath);
	}
}

void LandscapeDialog::reject()
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

	editor->RestoreInitialSettings();
	
	QDialog::reject();
}
