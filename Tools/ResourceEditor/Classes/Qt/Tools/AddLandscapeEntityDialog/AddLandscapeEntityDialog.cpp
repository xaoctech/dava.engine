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

#include "AddLandscapeEntityDialog.h"
#include <QLabel>
#include <QMessageBox>

#define  INIT_PATH_WIDGET(widgetName, widgetNum, widgetTitle, fileFilter) SelectPathWidgetBase* widgetName = new SelectPathWidgetBase(parent,resFolder,"", widgetTitle, fileFilter);\
widgetMap[widgetName] = DefaultInfo(widgetNum, landscape->GetTextureName(widgetNum));\
widgetName->setText(landscape->GetTextureName(widgetNum).GetAbsolutePathname());\
connect(widgetName, SIGNAL(PathSelected(DAVA::String)), this, SLOT(ValueChanged(DAVA::String)));

#define OPEN_TEXTURE_TITLE "Open texture"
#define TEXTURE_FILE_FILTER "PNG(*.png);; TEX(*.tex)"
#define HEIGHT_MAP_ID 0xABCD

AddLandscapeEntityDialog::AddLandscapeEntityDialog(Entity* landscapeEntity,  QWidget* parent)
		:BaseAddEntityDialog(parent)
{
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
	QLabel* label8 = new QLabel("Hight map:", parent);
	
	
	AddControlToUserContainer(label1);
	AddControlToUserContainer(colorTexutureWidget);
	AddControlToUserContainer(label2);
	AddControlToUserContainer(tileTexuture0Widget);
	AddControlToUserContainer(label3);
	AddControlToUserContainer(tileTexuture1Widget);
	AddControlToUserContainer(label4);
	AddControlToUserContainer(tileTexuture2Widget);
	AddControlToUserContainer(label5);
	AddControlToUserContainer(tileTexuture3Widget);
	AddControlToUserContainer(label6);
	AddControlToUserContainer(tileMaskWidget);
	AddControlToUserContainer(label7);
	AddControlToUserContainer(tiledTextureWidget);
	AddControlToUserContainer(label8);
	AddControlToUserContainer(heightMapWidget);
	
	additionalWidgets.push_back(label1);
	additionalWidgets.push_back(label2);
	additionalWidgets.push_back(label3);
	additionalWidgets.push_back(label4);
	additionalWidgets.push_back(label5);
	additionalWidgets.push_back(label6);
	additionalWidgets.push_back(label7);
	additionalWidgets.push_back(label8);
}

AddLandscapeEntityDialog::~AddLandscapeEntityDialog()
{
	RemoveAllControlsFromUserContainer();
	
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		delete it->first;
	}
	
	Q_FOREACH(QWidget* widget, additionalWidgets)
	{
		delete widget;
	}
}

void AddLandscapeEntityDialog::CleanupPathWidgets()
{
	for (DAVA::Map<SelectPathWidgetBase*,DefaultInfo>::iterator it = widgetMap.begin(); it != widgetMap.end(); ++it)
	{
		it->first->EraseWidget();
	}
}

void AddLandscapeEntityDialog::ValueChanged(String fileName)
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

void AddLandscapeEntityDialog::reject()
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
	
	QDialog::reject();
}
