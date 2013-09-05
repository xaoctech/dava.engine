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



#ifndef __RESOURCEEDITORQT__SYSTEMS_SETTINGS_EDITOR__
#define __RESOURCEEDITORQT__SYSTEMS_SETTINGS_EDITOR__

#include "DAVAEngine.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"

#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/GridSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/LandscapeEditorDrawSystem.h"
#include "Scene/System/HeightmapEditorSystem.h"
#include "Scene/System/TilemaskEditorSystem.h"
#include "Scene/System/CustomColorsSystem.h"
#include "Scene/System/VisibilityToolSystem.h"
#include "Scene/System/RulerToolSystem.h"
#include "Scene/System/StructureSystem.h"
#include "Scene/System/EditorParticlesSystem.h"
#include "Scene/System/EditorLightSystem.h"
#include "Scene/System/TextDrawSystem.h"

#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"

#include "Tools/QtPropertyEditor/QtPropertyItem.h"

#include <QPushButton>

typedef DAVA::Map<DAVA::String, std::pair<DAVA::uint32, bool> > STATE_FLAGS_MAP;

class SceneEditor2;

class SystemsSettingsEditor: public QtPropertyEditor
{
	Q_OBJECT
	
public:
	explicit SystemsSettingsEditor(QWidget* parent = 0);
	
	~SystemsSettingsEditor();
	
	void InitializeProperties();
	void RestoreInitialSettings();

	
protected slots:
	
	void HandleCustomColorBrushSize();

	void HandleCustomColorColorIndex();

	void HandleCameraMoveSpeed();
	
	void HandleCameraViewportRect();
	
	void HandleCollisionDrawMode();
	
	void HandleHoodModifMode();
	
	void HandleHoodPosition();
	
	void HandleHoodModifAxis();
	
	void HandleHoodScale();
	
	void HandleSelectionDrawMode();
	
	void HandlePivotPoint();
	
	void HandleEntityModifMode();
		
	void HandleEntityModifAxis();
	
	void HandleEntityLandscapeSnap();
	
	void HandleHightmapBrushSize();
	
	void HandleHightmapStrength();
	
	void HandleHightmapAverageStrength();
	
	void HandleHightmapDrawingType();
	
	void HandleTileMaskBrushSize();
	
	void HandleTileMaskStrength();
	
	void HandleTileTextureIndex();
	
	void HandleVisibToolBrushSize();
	
	void HandleLightCameraEnbled();
	
	void HandleRenderShadowRectColor();

protected slots:

	void ShowDialog();

protected:

	struct PropertyInfo
	{
		QtPropertyDataDavaVariant*	property;
		DAVA::VariantType			defaultValue;
		STATE_FLAGS_MAP *			flagsMap;
		PropertyInfo()
		{
			property = NULL;
			flagsMap = NULL;
		}
		PropertyInfo(QtPropertyDataDavaVariant* _property, DAVA::VariantType _defaultValue, STATE_FLAGS_MAP * _flagsMap = NULL)
		{
			property = _property;
			defaultValue = _defaultValue;
			flagsMap = _flagsMap;
		}
	};

	QPushButton * CreatePushBtn();
	
	DAVA::uint32 ResolveMapToUint(STATE_FLAGS_MAP& map);
	
	void InitMapWithFlag(STATE_FLAGS_MAP& map, DAVA::uint32 value);
	DAVA::String ResolveMapToString(STATE_FLAGS_MAP& map);
	
	
	DAVA::List<PropertyInfo> propertiesMap;

	SceneEditor2* sceneEditor;

	STATE_FLAGS_MAP selectionSysDrawStateMap;
	STATE_FLAGS_MAP collisionSysDrawStateMap;

	DAVA::Map<QPushButton * , PropertyInfo>  buttonsMap;
	
};
#endif /* defined(__RESOURCEEDITORQT__SYSTEMS_SETTINGS_EDITOR__) */