/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "RulerTool.h"

#include "../EditorScene.h"
#include "../SceneEditor/EditorBodyControl.h"

#include "../Qt/QtMainWindowHandler.h"
#include "../SceneEditor/HeightmapNode.h"

using namespace DAVA;


RulerTool::RulerTool(EditorBodyControl *parent)
    :   BaseObject()
{
    editorScene = NULL;
    landscape = NULL;
    heightmapNode = NULL;
    
    parentControl = parent;
    
    length = 0;
    landscapeSize = 0;
}

RulerTool::~RulerTool()
{
    DisableTool();
}

bool RulerTool::EnableTool(EditorScene *scene)
{
    DisableTool();
    
    length = 0;
    landscapeSize = 0;
    
    editorScene = SafeRetain(scene);
    if(scene)
    {
        Vector<LandscapeNode *>landscapes;
        scene->GetChildNodes(landscapes);
        
        if(0 < landscapes.size())
        {
            landscape = SafeRetain(landscapes[0]);
            Heightmap *heightmap = landscape->GetHeightmap();
            
            DVASSERT(heightmap && "Need Heightmap at landscape");
            landscapeSize = heightmap->Size();
            
            heightmapNode = new HeightmapNode(editorScene, landscape);
            editorScene->AddNode(heightmapNode);
        }
    }
    
    return (NULL != landscape);
}

void RulerTool::DisableTool()
{
    if(heightmapNode && editorScene)
    {
        editorScene->RemoveNode(heightmapNode);
    }
    
    SafeRelease(landscape);
    SafeRelease(editorScene);
    SafeRelease(heightmapNode);
}

bool RulerTool::Input(DAVA::UIEvent *touch)
{
    if(UIEvent::BUTTON_1 == touch->tid)
    {
        if(UIEvent::PHASE_ENDED == touch->phase)
        {
            Vector3 point;
            bool isIntersect = GetIntersectionPoint(touch->point, point);
            if(isIntersect)
            {
                bool commandKeyIsPressed = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT);
                if(commandKeyIsPressed)
                {
                    SetStartPoint(point);
                }
                else
                {
                    AddPoint(point);
                }
            }
            
        }
    }
    
    return true;
}

bool RulerTool::GetIntersectionPoint(const DAVA::Vector2 &touchPoint, DAVA::Vector3 &pointOnLandscape)
{
    DVASSERT(parentControl);

    Vector3 from, dir;
    parentControl->GetCursorVectors(&from, &dir, touchPoint);
    Vector3 to = from + dir * (float32)RAY_TRACING_DISTANCE;

    bool isIntersect = editorScene->LandscapeIntersection(from, to, pointOnLandscape);
    return isIntersect;
}

bool RulerTool::GetLandscapePoint(const Vector2 &touchPoint, Vector2 &landscapePoint)
{
    Vector3 point;
    bool isIntersect = GetIntersectionPoint(touchPoint, point);
    if(isIntersect)
    {
        AABBox3 box = landscape->GetBoundingBox();
        
        //TODO: use
        landscapePoint.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
        landscapePoint.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
    }
    
    return isIntersect;
}



void RulerTool::SetStartPoint(const DAVA::Vector3 &point)
{
    linePoints.clear();
    linePoints.push_back(point);

    length = 0;
    landscapeSize = 0;
    QtMainWindowHandler::Instance()->ShowStatusBarMessage(Format("Length: %f", length));
}

void RulerTool::AddPoint(const DAVA::Vector3 &point)
{
    if(0 < linePoints.size())
    {
        Vector3 prevPoint = *(linePoints.rbegin());
        length += GetLength(prevPoint, point);
        
        linePoints.push_back(point);
        
        QtMainWindowHandler::Instance()->ShowStatusBarMessage(Format("Length: %f", length));
    }
}

DAVA::float32 RulerTool::GetLength() const
{
    return length;
}

DAVA::float32 RulerTool::GetLength(const DAVA::Vector3 &startPoint, const DAVA::Vector3 &endPoint)
{
    float32 lineSize = 0.f;
    
    
    Vector3 prevPoint = startPoint;
    Vector3 prevLandscapePoint = LandscapePoint(prevPoint); //
    
    for(int32 i = 1; i <= APPROXIMATION_COUNT; ++i)
    {
        Vector3 point = startPoint + (endPoint - startPoint) * i / (float32)APPROXIMATION_COUNT;
        Vector3 landscapePoint = LandscapePoint(point); //
        
        lineSize += (landscapePoint - prevLandscapePoint).Length();
        
        prevPoint = point;
        prevLandscapePoint = landscapePoint;
    }
    
    return lineSize;
}

DAVA::Vector3 RulerTool::LandscapePoint(const DAVA::Vector3 &point)
{
    //TODO: need real code
    return point;
}



