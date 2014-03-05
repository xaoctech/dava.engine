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
#include "Commands2/ImageRegionCopyCommand.h"

GrassEditorSystem::GrassEditorSystem(Scene* scene)
: SceneSystem(scene)
, isEnabled(false)
, inDrawState(false)
, vegetationMap(NULL)
, vegetationMapCopy(NULL)
, curBrush(0x11)
, curLayer(0)
, curVegetation(NULL)
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
    SafeRelease(vegetationMap);
    SafeRelease(vegetationMapCopy);
    SafeRelease(curVegetation);
}

void GrassEditorSystem::Update(DAVA::float32 timeElapsed)
{ }

void GrassEditorSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
    if(isEnabled && NULL != vegetationMap)
    {
        UpdateCursorPos();

        if (event->tid == UIEvent::BUTTON_1)
	    {
		    Vector3 point;

		    switch(event->phase)
		    {
                case UIEvent::PHASE_BEGAN:
                    {
                        inDrawState = true;
                        affectedArea.Empty();
                        affectedArea.AddPoint(curCursorPos);
                        DrawGrass(curCursorPos);
                    }
				    break;

			    case UIEvent::PHASE_DRAG:
                    if(inDrawState)
                    {
                        affectedArea.AddPoint(curCursorPos);
                        DrawGrass(curCursorPos);
                    }
				    break;

			    case UIEvent::PHASE_ENDED:
                    if(inDrawState)
                    {
                        inDrawState = false;
                        DrawGrassEnd();
                    }
				    break;
		    }
	    }
    }
}

void GrassEditorSystem::ProcessCommand(const Command2 *command, bool redo)
{
    if(NULL != command)
    {
        int cmdId = command->GetId();
        if(cmdId == CMDID_IMAGE_REGION_COPY)
        {
            ImageRegionCopyCommand* imCmd = (ImageRegionCopyCommand *) command;

            BuildGrassCopy(DAVA::AABBox2(imCmd->pos, DAVA::Vector2(imCmd->pos.x + imCmd->orig->width - 1, imCmd->pos.y + imCmd->orig->height - 1)));
            ImageLoader::Save(vegetationMap, DAVA::FilePath("D:/grass.png"));
        }
    }
}

bool GrassEditorSystem::EnableGrassEdit(bool enable)
{
    bool ret = false;

    if(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == drawSystem->VerifyLandscape())
    {
        if(enable != isEnabled)
        {
            if(enable)
            {
                DAVA::VegetationRenderObject *veg = SearchVegetation(GetScene());
                curVegetation = SafeRetain(veg);

                if(NULL != veg && NULL !=veg->GetVegetationMap())
                {
                    isEnabled = true;
                    ret = true;

                    vegetationMap = SafeRetain(veg->GetVegetationMap());

                    selectionSystem->SetLocked(true);
                    modifSystem->SetLocked(true);

                    drawSystem->EnableCursor(vegetationMap->width);
                    drawSystem->SetCursorTexture(cursorTexture);
                    drawSystem->SetCursorSize(1);

                    BuildGrassCopy();
                }
            }
            else
            {
                selectionSystem->SetLocked(false);
                modifSystem->SetLocked(false);

                drawSystem->DisableCursor();
                drawSystem->DisableCustomDraw();

                SafeRelease(vegetationMap);
                SafeRelease(vegetationMapCopy);
                SafeRelease(curVegetation);

                isEnabled = false;
                ret = true;
            }
        }
    }

    return ret;
}

bool GrassEditorSystem::IsEnabledGrassEdit() const
{
    return isEnabled;
}

void GrassEditorSystem::UpdateCursorPos()
{
    Vector3 landPos;
    if(NULL != vegetationMap && collisionSystem->LandRayTestFromCamera(landPos))
    {
        Vector2 point(landPos.x, landPos.y);
        AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

        curCursorPos.x = (point.x - box.min.x) * vegetationMap->width / (box.max.x - box.min.x);
        curCursorPos.y = (point.y - box.min.y) * vegetationMap->height / (box.max.y - box.min.y);
        curCursorPos.x = (int32) curCursorPos.x;
        curCursorPos.y = (int32) curCursorPos.y;

        drawSystem->SetCursorPosition(curCursorPos);
    }
    else
    {
        curCursorPos = DAVA::Vector2(-100, -100);
    }

    drawSystem->SetCursorPosition(curCursorPos);
}

DAVA::VegetationRenderObject* GrassEditorSystem::SearchVegetation(DAVA::Entity *entity) const
{
    DAVA::VegetationRenderObject* ret = DAVA::GetVegetation(entity);

    if(NULL == ret && NULL != entity)
    {
        for(int i = 0; NULL == ret && i < entity->GetChildrenCount(); ++i)
        {
            ret = SearchVegetation(entity->GetChild(i));
        }
    }

    return ret;
}

void GrassEditorSystem::SetLayerVisible(uint8 layer, bool visible)
{
    // TODO:
    // ...
}

bool GrassEditorSystem::IsLayerVisible(uint8 layer) const
{
    // TODO:
    // ...

    return true;
}

void GrassEditorSystem::SetCurrentLayer(uint8 layer)
{
    DVASSERT(layer < 3);

    curLayer = layer;
}

uint8 GrassEditorSystem::GetCurrentLayer() const
{
    return curLayer;
}

void GrassEditorSystem::SetBrushType(uint8 type)
{
    curBrush |= ((type & 0x3) << 6);
}

uint8 GrassEditorSystem::GetBrushType() const
{
    return ((curBrush >> 6) & 0x3);
}

void GrassEditorSystem::SetBrushHeight(uint8 height)
{
    curBrush |= ((height & 0x3) << 4);
}

uint8 GrassEditorSystem::GetBrushHeight() const
{
    return ((curBrush >> 4) & 0x3);
}

void GrassEditorSystem::SetBrushDensity(uint8 density)
{
    curBrush |= (density & 0xf);
}

uint8 GrassEditorSystem::GetBrushDensity() const
{
    return (curBrush & 0xf);
}

void GrassEditorSystem::DrawGrass(DAVA::Vector2 pos)
{
    if(NULL != vegetationMap)
    {
        DVASSERT(pos.x < vegetationMap->width);
        DVASSERT(pos.y < vegetationMap->height);

        DAVA::uint32 offset = (pos.y * vegetationMap->width + pos.x);
        DAVA::uint32 ps = offset * Texture::GetPixelFormatSizeInBytes(vegetationMap->format);

        vegetationMap->data[ps + curLayer] = curBrush;
    }
}

void GrassEditorSystem::DrawGrassEnd()
{
    if(!affectedArea.IsEmpty() && NULL != vegetationMapCopy)
    {
        SceneEditor2 *sceneEditor = (SceneEditor2 *) GetScene();

        DAVA::Rect affectedRect = affectedArea.GetRect();
        DAVA::Image *orig = DAVA::Image::CopyImageRegion(vegetationMapCopy, affectedRect);
        sceneEditor->Exec(new ImageRegionCopyCommand(vegetationMap, affectedRect.GetPosition(), vegetationMap, affectedRect, orig));

        //BuildGrassCopy(affectedArea);
        //ImageLoader::Save(vegetationMap, DAVA::FilePath("D:/grass.png"));
    }
}

void GrassEditorSystem::BuildGrassCopy(DAVA::AABBox2 area)
{
    if(NULL != vegetationMap)
    {
        if(NULL == vegetationMapCopy)
        {
            vegetationMapCopy = DAVA::Image::CreateFromData(vegetationMap->width, vegetationMap->height, vegetationMap->format, vegetationMap->data);
        }
        else
        {
            DAVA::Rect r = area.GetRect();
            vegetationMapCopy->InsertImage(vegetationMap, r.GetPosition(), r);
        }
    }
}

DAVA::VegetationRenderObject* GrassEditorSystem::GetCurrentVegetationObject() const
{
    return curVegetation;
}
