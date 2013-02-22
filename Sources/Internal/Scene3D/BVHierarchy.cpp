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

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Scene3D/BVHierarchy.h"
#include "Scene3D/Scene.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Debug/Stats.h"

namespace DAVA
{
    
BVHierarchy::BVHierarchy()
{
}
    
BVHierarchy::~BVHierarchy()
{

}
    
void BVHierarchy::ChangeScene(Scene * _scene)
{
    scene = _scene;
}   

void BVHierarchy::RegisterNode(SceneNode * node)
{
    MeshInstanceNode * meshInstance = dynamic_cast<MeshInstanceNode*>(node);
    if (meshInstance)
    {
        //Logger::Debug("Register mesh: %p cn: %s", meshInstance, node->GetClassName().c_str());
        meshInstances.push_back(meshInstance);
    }
}
    
void BVHierarchy::UnregisterNode(SceneNode * node)
{
    MeshInstanceNode * meshInstance = dynamic_cast<MeshInstanceNode*>(node);
    if (meshInstance)
    {
        //Logger::Debug("Unregister mesh: %p cn: %s", meshInstance, node->GetClassName().c_str());
        uint32 size = (uint32)meshInstances.size();
        uint32 pos = 0;
        for (pos = 0; pos < size; ++pos)
        {
            if (meshInstances[pos] == meshInstance)
            {
                meshInstances[pos] = meshInstances[size - 1];
                meshInstances.pop_back();
                break;
            }
        }
    }
}

void BVHierarchy::Cull()
{
    // Bruce force frustum culling
    int32 objectsCulled = 0;
    
    Frustum * frustum = scene->GetClipCamera()->GetFrustum();
    uint32 size = meshInstances.size();
    for (uint32 pos = 0; pos < size; ++pos)
    {
        MeshInstanceNode * node = meshInstances[pos];
        node->RemoveFlag(SceneNode::NODE_CLIPPED_THIS_FRAME);
        //Logger::Debug("Cull Node: %s rc: %d", node->GetFullName().c_str(), node->GetRetainCount());
        if (!frustum->IsInside(node->GetWorldTransformedBox()))
        {
            node->AddFlag(SceneNode::NODE_CLIPPED_THIS_FRAME);
            objectsCulled++;
        }
    }
    //Logger::Debug("Objects: %d Objects Drawn: %d", size, (size - objectsCulled));
}

  
};




