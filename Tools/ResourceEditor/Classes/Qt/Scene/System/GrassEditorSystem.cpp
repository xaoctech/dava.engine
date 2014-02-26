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



#include "GrassEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/GrassEditorProxy.h"
#include "Scene/SceneSignals.h"

GrassEditorSystem::GrassEditorSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/squareCursor.tex");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);

	collisionSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditor2 *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditor2 *) GetScene())->modifSystem;
	drawSystem = ((SceneEditor2 *) GetScene())->landscapeEditorDrawSystem;
}

GrassEditorSystem::~GrassEditorSystem()
{
	SafeRelease(cursorTexture);
}

void GrassEditorSystem::Update(DAVA::float32 timeElapsed)
{ }

void GrassEditorSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
    if(enabled)
    {
	    UpdateCursorPosition(128);

	    if (UIEvent::PHASE_KEYCHAR == event->phase)
	    {
	    }
	    else if (event->tid == UIEvent::BUTTON_1)
	    {
		    Vector3 point;

		    switch(event->phase)
		    {
			    case UIEvent::PHASE_BEGAN:
				    break;

			    case UIEvent::PHASE_DRAG:
				    break;

			    case UIEvent::PHASE_ENDED:
				    break;
		    }
	    }
    }
}

void GrassEditorSystem::EnableGrassEdit(bool enable)
{
    if(enable != enabled)
    {
        enabled = enable;

        if(enable)
        {
            selectionSystem->SetLocked(true);
            modifSystem->SetLocked(true);

            drawSystem->EnableCursor(128);
            drawSystem->SetCursorTexture(cursorTexture);
            drawSystem->SetCursorSize(1);
        }
        else
        {
            selectionSystem->SetLocked(false);
            modifSystem->SetLocked(false);

            drawSystem->DisableCursor();
            drawSystem->DisableCustomDraw();
        }
    }
}

void GrassEditorSystem::UpdateCursorPosition(int32 landscapeSize)
{
	Vector3 landPos;
	if (collisionSystem->LandRayTestFromCamera(landPos))
	{
		Vector2 point(landPos.x, landPos.y);

		//point.x = (float32)((int32)point.x);
		//point.y = (float32)((int32)point.y);

		AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

		cursorPosition.x = (point.x - box.min.x) * landscapeSize / (box.max.x - box.min.x);
		cursorPosition.y = (point.y - box.min.y) * landscapeSize / (box.max.y - box.min.y);
		cursorPosition.x = (int32)cursorPosition.x;
		cursorPosition.y = (int32)cursorPosition.y;

		drawSystem->SetCursorPosition(cursorPosition);
	}
	else
	{
		// hide cursor
		drawSystem->SetCursorPosition(DAVA::Vector2(-100, -100));
	}
}
