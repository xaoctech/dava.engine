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

#include "LandscapesController.h"

#include "EditorHeightmap.h"
#include "NotPassableTerrain.h"
#include "LandscapeRenderer.h"

using namespace DAVA;

LandscapesController::LandscapesController()
    :   BaseObject()
{
    scene = NULL;
    renderedHeightmap = NULL;
    notPassableTerrain = NULL;
    landscapeRenderer = NULL;
    
    savedLandscape = NULL;
    savedHeightmap = NULL;
}

LandscapesController::~LandscapesController()
{
    ReleaseScene();
}

void LandscapesController::SetScene(DAVA::Scene *scene)
{
    ReleaseScene();

    this->scene = SafeRetain(scene);
    
    if(scene)
    {
        Vector<LandscapeNode *>landscapes;
        scene->GetChildNodes(landscapes);
        
        if(0 < landscapes.size())
        {
            SaveLandscape(landscapes[0]);
        }
    }
}

void LandscapesController::ReleaseScene()
{
    if(notPassableTerrain && notPassableTerrain->GetParent())
    {
        notPassableTerrain->GetParent()->RemoveNode(notPassableTerrain);
    }
    SafeRelease(notPassableTerrain);

    
    SafeRelease(renderedHeightmap);
    SafeRelease(landscapeRenderer);
    

    if(savedHeightmap && savedLandscape)
    {
        savedLandscape->SetHeightmap(savedHeightmap);
    }
    SafeRelease(savedHeightmap);
    SafeRelease(savedLandscape);
    
    
    SafeRelease(scene);
}

void LandscapesController::SaveLandscape(DAVA::LandscapeNode *landscape)
{
    SafeRelease(savedHeightmap);
    SafeRelease(savedLandscape);
    
    if(landscape)
    {
        savedLandscape = SafeRetain(landscape);
        savedHeightmap = SafeRetain(landscape->GetHeightmap());
    }
}


void LandscapesController::ToggleNotPassableLandscape()
{
    DVASSERT(scene && "Need set scene before");
    
    if(notPassableTerrain)
    {
        HideEditorLandscape(notPassableTerrain);
        notPassableTerrain = NULL;
    }
    else
    {
        notPassableTerrain = new NotPassableTerrain();
        bool showed = ShowEditorLandscape(notPassableTerrain);
        if(!showed)
        {
            SafeRelease(notPassableTerrain);
        }
    }
}

bool LandscapesController::ShowEditorLandscape(EditorLandscapeNode *displayingLandscape)
{
    Vector<LandscapeNode *>landscapes;
    scene->GetChildNodes(landscapes);
    
    if(1 != landscapes.size())
    {
        Logger::Error("[LandscapesController::ShowEditorLandscape] Can be only one landscape");
        return false;
    }

    LandscapeNode *landscape = landscapes[0];
    displayingLandscape->SetLandscape(landscape);
    
    if(landscapeRenderer)
    {
        displayingLandscape->SetHeightmap(landscape->GetHeightmap());
    }
    else
    {
        EditorHeightmap *heightmap = new EditorHeightmap(landscape->GetHeightmap());
        displayingLandscape->SetHeightmap(heightmap);

        landscapeRenderer = new LandscapeRenderer(heightmap, landscape->GetBoundingBox());
        SafeRelease(heightmap);
    }
    
    displayingLandscape->SetRenderer(landscapeRenderer);
    
    SceneNode *parentNode = landscape->GetParent();
    parentNode->RemoveNode(landscape);
    parentNode->AddNode(displayingLandscape);

    return true;
}

void LandscapesController::HideEditorLandscape(EditorLandscapeNode *hiddingLandscape)
{
    SceneNode *parentNode = hiddingLandscape->GetParent();
    LandscapeNode *restoredLandscape = SafeRetain(hiddingLandscape->GetEditedLandscape());
    
    if(parentNode)
    {
        parentNode->RemoveNode(hiddingLandscape);
        parentNode->AddNode(restoredLandscape);
    }
    
    if(NeedToKillRenderer(restoredLandscape))
    {
        SafeRelease(landscapeRenderer);
    }
    
    SafeRelease(hiddingLandscape);
    SafeRelease(restoredLandscape);
}


bool LandscapesController::NeedToKillRenderer(DAVA::LandscapeNode *landscapeForDetection)
{
    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(landscapeForDetection);
    return (NULL == editorLandscape);
}





