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
#ifndef __DAVAENGINE_SCENE3D_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"

namespace DAVA 
{
    
class SceneNode;
class Component
{
public:
    enum eType
    {
        TRANSFORM_COMPONENT = 0,
        RENDER_COMPONENT,
        DEBUG_RENDER_COMPONENT, 
		LOD_COMPONENT,
        UPDATE_COMPONENT,
        CAMERA_COMPONENT,
        LIGHT_COMPONENT,
		PARTICLE_EMITTER_COMPONENT,
        ANIMATION_COMPONENT,
        COLLISION_COMPONENT,    // multiple instances
        PHYSICS_COMPONENT,
        ACTION_COMPONENT,       // actions, something simplier than scripts that can influence logic, can be multiple
        SCRIPT_COMPONENT,       // multiple instances, not now, it will happen much later.
        COMPONENT_COUNT,
    };

	Component();
    
    virtual ~Component() {};
    virtual uint32 GetType() = 0;
    virtual Component * Clone() = 0;

	void SetEntity(SceneNode * entity);
protected:
    SceneNode * entity;

public:
	INTROSPECTION_EMPTY(Component);
};

    
#define IMPLEMENT_COMPONENT_TYPE(TYPE) virtual uint32 GetType() { return TYPE; }; 

};
#endif //__DAVAENGINE_SCENE3D_COMPONENT_H__
