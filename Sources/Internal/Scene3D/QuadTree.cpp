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
#include "Scene3D/QuadTree.h"
#include "Scene3D/Scene.h"
#include "Scene3D/MeshInstanceNode.h"

namespace DAVA
{
QuadTreeNode::QuadTreeNode(const AABBox3 & _bbox)
    : bbox(_bbox)
{
    for (int32 k = 0; k < 4; ++k)
        children[k] = 0;
}
    
QuadTreeNode::~QuadTreeNode()
{
    const Vector<MeshInstanceNode*>::iterator & end = objectsInside.end(); 
    for (Vector<MeshInstanceNode*>::iterator it = objectsInside.begin();  it != end; ++it)
    {
        MeshInstanceNode * node = *it;
        SafeRelease(node);
    }
    for (int32 k = 0; k < 4; ++k)
        SafeDelete(children[k]);
}

    
QuadTree::QuadTree()
{
    
}
    
QuadTree::~QuadTree()
{
    SafeDelete(head);
}

void QuadTree::Build(Scene * scene)
{
    List<MeshInstanceNode*> meshNodes;
    scene->GetChildNodes(meshNodes);
    
    AABBox3 sceneBoundingBox = scene->GetWTMaximumBoundingBox();
    head = new QuadTreeNode(sceneBoundingBox);
    
    BuildRecursive(head, meshNodes);
}

void QuadTree::Update(float32 timeElapsed)
{
    
}

void QuadTree::Draw()
{
    
}

void QuadTree::BuildRecursive(QuadTreeNode * node, List<MeshInstanceNode*> & meshNodes)
{
    //meshNodes
    AABBox3 bbox = node->GetBoundingBox();
    
    AABBox3 childBoxes[4]; 
    Vector3 halfSize = (bbox.max - bbox.min) / 2.0f;
    childBoxes[0] = AABBox3(Vector3(bbox.min.x, bbox.min.y, bbox.min.z), Vector3(bbox.min.x + halfSize.x, bbox.max.y, bbox.min.z + halfSize.z));
    childBoxes[1] = AABBox3(Vector3(bbox.min.x + halfSize.x, bbox.min.y, bbox.min.z), Vector3(bbox.max.x, bbox.max.y, bbox.min.z + halfSize.z));
    childBoxes[2] = AABBox3(Vector3(bbox.min.x, bbox.min.y, bbox.min.z + halfSize.z), Vector3(bbox.min.x + halfSize.x, bbox.max.y, bbox.max.z));
    childBoxes[3] = AABBox3(Vector3(bbox.min.x + halfSize.x, bbox.min.y, bbox.min.z + halfSize.z), Vector3(bbox.max.z, bbox.max.y, bbox.max.z));
    
    int32 childCount[4] = {0, 0, 0, 0};
    List<MeshInstanceNode*> childLists[4];
    
    for (List<MeshInstanceNode*>::iterator it = meshNodes.begin(); it != meshNodes.end();)
    {
        MeshInstanceNode * node = *it;
        bool nodeIn = false;
        for (int k = 0; k < 4; ++k)
        {
            const AABBox3 & bbox = node->GetBoundingBox();
            if (bbox.IsInside(childBoxes[k]))
            {
                childCount[k]++;
                childLists[k].push_back(node);
                nodeIn = true;
                break;
            }
        }
        if (nodeIn)
        {
            it = meshNodes.erase(it);
        }else
        {
            it++;
        }
        
    }
    
    //for (each new node where number of nodes inside is not 0 build recursively) 
    for (int k = 0; k < 4; ++k)
    {
        if (childCount[k] > 0)
        {
            node->children[k] = new QuadTreeNode(childBoxes[k]);
            BuildRecursive(node->children[k], childLists[k]);
        }
    }    
    
    // all objects that are not in childs remains in this node
    for (List<MeshInstanceNode*>::iterator it = meshNodes.begin(); it != meshNodes.end(); ++it)
    {
        node->objectsInside.push_back(SafeRetain(*it));
    }
}

    
    
};




