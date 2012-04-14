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
#ifndef __DAVAENGINE_QUADTREE_H__
#define __DAVAENGINE_QUADTREE_H__

#include "Base/BaseObject.h"
#include "Base/StaticSingleton.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNodeAnimationKey.h"
#include "Scene3D/BVHierarchy.h"
#include <deque>

namespace DAVA
{
class Material;
class MeshInstanceNode;
class QuadTree;
    
class QuadTreeNode 
{
public:
    // be careful it's not a BaseObject to avoid unnecessary data store
    QuadTreeNode(const AABBox3 & bbox);
    virtual ~QuadTreeNode();
    
    inline const AABBox3 & GetBoundingBox() { return bbox; };
    
private:
    AABBox3 bbox;
    QuadTreeNode * children[4];
    Vector<MeshInstanceNode*> objectsInside;
    friend class QuadTree;
};

class QuadTree : public BVHierarchy
{
public:
    QuadTree();
    virtual ~QuadTree();
    
    virtual void Build(Scene * scene);
    virtual void Update(float32 timeElapsed);
    virtual void Draw(); 
private:
    void BuildRecursive(QuadTreeNode * node, List<MeshInstanceNode*> & meshNodes);
    
    QuadTreeNode * head;
    Map<Material*, Set<MeshInstanceNode*> > nodesForRender;
};

};

#endif // __DAVAENGINE_QUADTREE_H__





