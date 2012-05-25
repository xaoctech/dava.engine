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
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTR ACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "LandscapeTest.h"

LandscapeTest::LandscapeTest(const String &testName, LandscapeNode::eTiledShaderMode mode)
    :   TestTemplate(testName)
    ,   shaderMode(mode)
{
    testCounter = 0;
    startTime = 0;
    
    for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
    {
        textures[i] = NULL;
    }
}

void LandscapeTest::LoadResources()
{
	LandscapeNode * land = new LandscapeNode();  
    SafeRelease(land);
    
    Scene *scene = new Scene();
    scene->AddNode(scene->GetRootNode("~res:/3d/LandscapeTest/landscapetest.sc2")); 

    Camera *cam = (Camera *)scene->FindByName("TestCamera");
    scene->AddCamera(cam); 
    scene->SetCurrentCamera(cam);
    
    land = (LandscapeNode *)scene->FindByName("Landscape"); 
    if(land)
    {
        if(LandscapeNode::TILED_MODE_COUNT == shaderMode)
        {
            for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
            {
                Texture *t = land->GetTexture((LandscapeNode::eTextureLevel)i);
                if(t)
                {
                    textures[i] = Sprite::CreateFromTexture(t, 0, 0, t->GetWidth(), t->GetHeight());
                    RegisterFunction(this, &LandscapeTest::DrawSprite, "TestLandscapeTexture", TEST_FRAMES_COUNT/2, textures[i]);
                }
            }
        }
        else 
        {
            land->SetTiledShaderMode(shaderMode);
            UI3DView *sceneView = new UI3DView(Rect(0, 0, GetSize().x, GetSize().y));
            sceneView->SetScene(scene);
            AddControl(sceneView);
            SafeRelease(sceneView);
        }
    }
    
    SafeRelease(scene);
    
    testCounter = 0;
    startTime = 0;
}

void LandscapeTest::UnloadResources()
{
    for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
    {
        SafeRelease(textures[i]);
    }

    RemoveAllControls();

    testCounter = 0;
}

void LandscapeTest::RunTests()
{
    if(LandscapeNode::TILED_MODE_COUNT == shaderMode)
    {
        TestTemplate<LandscapeTest>::RunTests();
    }
    else 
    {
        if(LOADING_FRAMES_COUNT == testCounter)
        {
            startTime = SystemTimer::Instance()->AbsoluteMS();
        }
        else if(FULL_FRAMES_COUNT == testCounter)
        {
            uint64 endTime = SystemTimer::Instance()->AbsoluteMS();
            uint64 testTime = endTime - startTime;
            
            int32 fps = (int32)((float32)TEST_FRAMES_COUNT / ((float32)testTime / 1000.0f));
            
            PerfFuncData data = {0};
            data.name = screenName;
            data.startTime = startTime;
            data.endTime = endTime;
            data.maxTime = fps;
            data.runCount = TEST_FRAMES_COUNT;
            runIndex = 0;
            
            WriteLog(&data);
            
            Logger::Debug("%s %s, fps = %d", screenName.c_str(), data.name.c_str(), fps);
            
            GameCore::Instance()->TestFinished();
        }
        ++testCounter;
    }
}

void LandscapeTest::DrawSprite(PerfFuncData * data)
{
    Sprite *sprite = (Sprite *)data->userData;
    
	sprite->Reset();
    sprite->SetFrame(0);
    sprite->SetPosition(GetSize().x - sprite->GetWidth(), GetSize().y - sprite->GetHeight());
//    float32 minSize = Min(GetSize().x, GetSize().y);
//    sprite->SetScaleSize(minSize, minSize);
    sprite->Draw();
}


void LandscapeTest::Draw(const UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearDepthBuffer();

    TestTemplate<LandscapeTest>::Draw(geometricData);
}
