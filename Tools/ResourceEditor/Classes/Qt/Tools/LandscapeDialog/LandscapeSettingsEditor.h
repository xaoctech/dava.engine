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



#ifndef __RESOURCEEDITORQT__LANDSCAPE_SETTINGS_EDITOR__
#define __RESOURCEEDITORQT__LANDSCAPE_SETTINGS_EDITOR__

#include "DAVAEngine.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "DockProperties/PropertyEditorDialog.h"
#include <QPushButton>

class LandscapeSettingsEditor: public PropertyEditorDialog
{
	Q_OBJECT
	
public:
	explicit LandscapeSettingsEditor(QWidget* parent = 0);
	
	~LandscapeSettingsEditor();
	
	void InitializeProperties(Entity* landscapeEntity);
	
	void RestoreInitialSettings();
	
	void SetNode(Entity* landscapeEntity);
	
signals:
	
	void TileModeChanged(int newValue);
	
protected slots:
	
	void HandleStaticLightEnabled();
	
	void HandleCastShadows();
	
	void HandleReceiveShadows();
	
	void HandleFogEnabled();
	
	void HandleFogDensity();
	
	void HandleFogColor();
	
	void HandleLandSize();
	
	void HandleLandHeight();
	
	void HandleTileColor0();
	
	void HandleTileColor1();
	
	void HandleTileColor2();

	void HandleTileColor3();
	
	void HandleTileMode();
	
	void HandleLightmapSize();
	
	void HandleTexture0TilingX();
	void HandleTexture0TilingY();
	void HandleTexture1TilingX();
	void HandleTexture1TilingY();
	void HandleTexture2TilingX();
	void HandleTexture2TilingY();
	void HandleTexture3TilingX();
	void HandleTexture3TilingY();

protected:

	void AppleNewLandscapeSize();
	
	struct PropertyInfo
	{
		QtPropertyDataDavaVariant*	property;
		QVariant					defaultValue;
		PropertyInfo()
		{
			property = NULL;
		}
		PropertyInfo(QtPropertyDataDavaVariant* _property, QVariant _defaultValue)
		{
			property = _property;
			defaultValue = _defaultValue;
		}
	};

	DAVA::List<PropertyInfo> propertiesMap;
	
	DAVA::KeyedArchive* propertyList;
	DAVA::Landscape*	landscape;
	DAVA::Entity*		landscapeEntity;
	DAVA::Vector3		size;

	DAVA::Vector<DAVA::String> tiledModes;
	
};
#endif /* defined(__RESOURCEEDITORQT__LANDSCAPE_SETTINGS_EDITOR__) */