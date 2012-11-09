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

#include "SceneFileTest.h"



SceneFileTest::SceneFileTest()
: TestTemplate<SceneFileTest>("SceneFileTest")
{
    RegisterFunction(this, &SceneFileTest::TestFunction, String("Test of Scene Files V2"), NULL);
}

void SceneFileTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
}


void SceneFileTest::UnloadResources()
{
    RemoveAllControls();
}

void SceneFileTest::TestFunction(PerfFuncData * data)
{
//    String loadingPathname = String("~res:/3d/Maps/test/treetest/TEST_2.sc2");
//    String savingPathname = String("~doc:/3d/Maps/test/treetest/TEST_2.sc2");
    String loadingPathname = String("~doc:/3d/Maps/test/treetest/TEST_2.sc2");
    String savingPathname = String("~doc:/3d/Maps/test/treetest/TEST_2_1.sc2");
    
    Scene *scene = LoadScene(loadingPathname);
    SaveScene(savingPathname, scene);
    SafeRelease(scene);

    
    File *file1 = File::Create(loadingPathname, File::OPEN | File::READ);
    File *file2 = File::Create(savingPathname, File::OPEN | File::READ);

    if(file1 && file2)
    {
        uint32 size1 = file1->GetSize();
        uint32 size2 = file2->GetSize();
        
        if(size1 == size2)
        {
            uint8 *data1 = new uint8[size1];
            uint8 *data2 = new uint8[size2];
            
            file1->Read(data1, size1);
            file2->Read(data2, size2);
            
            int32 cmpResult = memcmp(data1, data2, size1);
            
            data->testData.message = String(Format("Data compare result is (%d)", cmpResult));
            TEST_VERIFY(0 == cmpResult);
            
            SafeDeleteArray(data1);
            SafeDeleteArray(data2);
        }
        else
        {
            data->testData.message = String("Files have different size");
            TEST_VERIFY(false);
        }
    }
    else
    {
        data->testData.message = String("Can't open files");
        TEST_VERIFY(false);
    }
    
    SafeRelease(file1);
    SafeRelease(file2);
}

Scene * SceneFileTest::LoadScene(const String &pathname)
{
    //Load scene with *.sc2
    Scene *scene = new Scene();
    SceneNode *rootNode = scene->GetRootNode(pathname);
    if(rootNode)
    {
        Vector<SceneNode*> tempV;
		tempV.reserve(rootNode->GetChildrenCount());
        
		for (int32 ci = 0; ci < rootNode->GetChildrenCount(); ++ci)
		{
			tempV.push_back(rootNode->GetChild(ci));
		}
        for (int32 ci = 0; ci < (int32)tempV.size(); ++ci)
        {
            //рут нода это сама сцена в данном случае
            scene->AddNode(tempV[ci]);
        }
    }
    
    return scene;
}

void SceneFileTest::SaveScene(const String &pathname, DAVA::Scene *scene)
{
    String folder, filename;
    FileSystem::Instance()->SplitPath(pathname, folder, filename);
    
    folder = FileSystem::Instance()->SystemPathForFrameworkPath(folder);
    FileSystem::Instance()->CreateDirectory(folder, true);
    
    SceneFileV2 * outFile = new SceneFileV2();
    outFile->EnableSaveForGame(false);
    outFile->EnableDebugLog(false);
    outFile->SaveScene(pathname, scene);
    SafeRelease(outFile);
}


void SceneFileTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.f, 0.0f, 0.f, 1.f);

    TestTemplate<SceneFileTest>::Draw(geometricData);
}



