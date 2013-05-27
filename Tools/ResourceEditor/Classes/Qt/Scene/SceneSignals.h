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

#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include <QObject>

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditorProxy.h"

// framework
#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

class SceneEditorProxy;

class SceneSignals : public QObject, public DAVA::StaticSingleton<SceneSignals>
{
	Q_OBJECT

signals:
	void Opened(SceneEditorProxy *scene);
	void Closed(SceneEditorProxy *scene);

	void Loaded(SceneEditorProxy *scene);
	void Saved(SceneEditorProxy *scene);

	void Activated(SceneEditorProxy *scene);
	void Deactivated(SceneEditorProxy *scene);

	void Selected(SceneEditorProxy *scene, DAVA::Entity *entity);
	void Deselected(SceneEditorProxy *scene, DAVA::Entity *entity);

	void MouseOver(SceneEditorProxy *scene, const EntityGroup *entities);
	void MouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities);

public:
	void EmitOpened(SceneEditorProxy *scene) { emit Opened(scene); }
	void EmitClosed(SceneEditorProxy *scene) { emit Closed(scene); }

	void EmitLoaded(SceneEditorProxy *scene) { emit Loaded(scene); }
	void EmitSaved(SceneEditorProxy *scene) { emit Saved(scene); }

	void EmitActivated(SceneEditorProxy *scene) { emit Activated(scene); }
	void EmitDeactivated(SceneEditorProxy *scene) { emit Deactivated(scene); }

	void EmitSelected(SceneEditorProxy *scene, DAVA::Entity *entity) { emit Selected(scene, entity); }
	void EmitDeselected(SceneEditorProxy *scene, DAVA::Entity *entity)  { emit Deselected(scene, entity); }

	void EmitMouseOver(SceneEditorProxy *scene, const EntityGroup *entities) { emit MouseOver(scene, entities); }
	void EmitMouseOverSelection(SceneEditorProxy *scene, const EntityGroup *entities) { emit MouseOverSelection(scene, entities); }
};

#endif // __SCENE_MANAGER_H__
