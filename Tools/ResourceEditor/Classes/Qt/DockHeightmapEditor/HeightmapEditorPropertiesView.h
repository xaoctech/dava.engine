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



#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORPROPERTIESVIEW__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORPROPERTIESVIEW__

#include <QWidget>
#include <QDockWidget>
#include "DAVAEngine.h"
#include "Qt/Scene/System/HeightmapEditorSystem.h"

using namespace DAVA;

class SceneEditor2;

namespace Ui
{
	class HeightmapEditorPropertiesView;
}

class HeightmapEditorPropertiesView: public QWidget
{
	Q_OBJECT

public:
	explicit HeightmapEditorPropertiesView(QWidget* parent = 0);
	~HeightmapEditorPropertiesView();

	void Init();
	void InitBrushImages();

private slots:
	void SceneActivated(SceneEditor2* scene);
	void SceneDeactivated(SceneEditor2* scene);
	void SetDropperHeight(SceneEditor2* scene, double height);
	void HeightUpdatedManually();

	void Toggle();
	void SetBrushSize(int brushSize);
	void SetToolImage(int toolImage);
	void SetRelativeDrawing();
	void SetAverageDrawing();
	void SetAbsoluteDrawing();
	void SetAbsDropDrawing();
	void SetDropper();
	void SetHeightmapCopyPaste();
	void SetStrength(int strength);
	void SetAverageStrength(int averageStrength);
	void SetCopyPasteHeightmap(int state);
	void SetCopyPasteTilemask(int state);

private:
	static const int DEF_BRUSH_MIN_SIZE = 3;
	static const int DEF_BRUSH_MAX_SIZE = 40;
	static const int DEF_STRENGTH_MAX_VALUE = 30;
	static const int DEF_AVERAGE_STRENGTH_MIN_VALUE = 0;
	static const int DEF_AVERAGE_STRENGTH_MAX_VALUE = 60;

	Ui::HeightmapEditorPropertiesView* ui;
	SceneEditor2* activeScene;
	QAction* toolbarAction;
	QDockWidget* dockWidget;

	void SetWidgetsState(bool enabled);
	void BlockAllSignals(bool block);
	void UpdateFromScene(SceneEditor2* scene);
	void SetDrawingType(HeightmapEditorSystem::eHeightmapDrawType type);

	float32 StrengthFromInt(int32 val);
	int32 IntFromStrength(float32 strength);

	float32 AverageStrengthFromInt(int32 val);
	int32 IntFromAverageStrength(float32 averageStrength);

	int32 BrushSizeFromInt(int32 val);
	int32 IntFromBrushSize(int32 brushSize);

	void UpdateRadioState(HeightmapEditorSystem::eHeightmapDrawType type);
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORPROPERTIESVIEW__) */
