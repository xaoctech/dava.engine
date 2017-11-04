/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
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

    Revision History:
 * Created by Yury Danilov
   =====================================================================================*/
#include "ServerSPT.h"
#include "physics2/bsp.hpp"
#include "speedtree/speedtree_collision.hpp"
#include "Model.h"

ServerSPT::ServerSPT(const DAVA::String& _fileName, const DAVA::String& _mapName, DAVA::Entity* model)
    : modelName(_fileName)
    , mapName(_mapName)
    , density(DEFAULT_DENSITY)
{
    using namespace DAVA;

    Logger::Info("SpeedTree %s", _fileName.c_str());
    ReadCustomProperties(model);
    FilePath fileName(_fileName);
    modelName = fileName.GetBasename();

    srand((unsigned int)13);
    Scene* scene = new Scene();
    if (model != 0)
    {
        clone = model->Clone();
        clone->SetLocalTransform(Matrix4::IDENTITY);
        scene->AddNode(clone);
        scene->Update(0.1f);

        scene->transformSystem->Process(0.1f);
        scene->lodSystem->Process(0.1f);
        pBSP = createBSPTree(clone, _mapName + "\\speedTree\\" + modelName);
        PrepareDestructible();
    }
    else
    {
        isWrongModel = true;
        Logger::Warning("Failed to load model file %s", _fileName.c_str());
    }
    SafeRelease(scene);
}

void ServerSPT::ReadCustomProperties(DAVA::Entity* model)
{
    DAVA::KeyedArchive* p = DAVA::GetCustomPropertiesArchieve(model);
    if (p)
    {
        health = p->GetInt32("Health", 5);
        fallType = p->GetInt32("FallType", 0);
        density = p->GetFloat("Density", DEFAULT_DENSITY);
    }
}

void ReadBuffer_(char* buffer, int bufferSize, DAVA::String fileName)
{
    DAVA::File* f = DAVA::File::Create(fileName, DAVA::File::OPEN | DAVA::File::READ);
    DAVA::uint32 res = f->Read(buffer, bufferSize);
    buffer[res] = 0;
    DAVA::SafeRelease(f);
}

void ServerSPT::PrepareDestructible()
{
    using namespace DAVA;

    char content[8192];
    char result[8192];

    String path = "out\\tree\\";
    ReadBuffer_(content, sizeof(content), "~res:/modelTemplate\\destructibleTree.xml");

    _snprintf(result, sizeof(result), content,
              (mapName + "speedTree/" + modelName + ".spt").c_str(), health, density, Model::GetPhysicsParams(
                                                                                      fallType)
                                                                                      .c_str());
    File* f = File::Create(path + modelName + ".xml", File::CREATE | File::WRITE);
    f->Write(result, strlen(result));
    SafeRelease(f);
}
