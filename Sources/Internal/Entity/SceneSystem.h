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


#ifndef __DAVAENGINE_SCENE3D_SCENESYSTEM_H__
#define __DAVAENGINE_SCENE3D_SCENESYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{

class Entity;
class Scene;    
class Component;

class SceneSystem
{
public:
    SceneSystem(Scene * scene);
    virtual ~SceneSystem();
    
    inline void SetRequiredComponents(uint32 requiredComponents);
    inline uint32 GetRequiredComponents() const;
    
    /**
        \brief  This function is called when any entity added to scene.
                It sorts out is entity has all necessary components and we need to call AddEntity.
        \param[in] entity entity we've just added
     */
    virtual void AddEntityIfRequired(Entity * entity);
    /**
        \brief  This function is called when any entity removed from scene.
                It sorts out is entity has all necessary components and we need to call RemoveEntity.
        \param[in] entity entity we've just removed
     */
    virtual void RemoveEntityIfNotRequired(Entity * entity);
    
    /**
        \brief This function is called only when entity has all required components
        \param[in] entity entity we've just added
     */
    virtual void AddEntity(Entity * entity);
    
    /**
        \brief This function is called only when entity had all required components, and dont have them anymore. 
        \param[in] entity entity we've just removed
     */
    virtual void RemoveEntity(Entity * entity);
	
    virtual void AddComponent(Entity * entity, Component * component);
	virtual void RemoveComponent(Entity * entity, Component * component);
    
    virtual void SceneDidLoaded();

    virtual void ImmediateEvent(Entity * entity, uint32 event);
    
    virtual void Process(float32 timeElapsed);

	virtual void SetLocked(bool locked);
	virtual bool IsLocked();
	
	virtual void SetParent(DAVA::Entity *entity, DAVA::Entity *parent);

protected:
	inline Scene * GetScene() const;

private:
    uint32 requiredComponents;
	Scene * scene;

	bool locked;
};
    
// Inline
inline Scene * SceneSystem::GetScene() const
{
    return scene;
}

inline void SceneSystem::SetRequiredComponents(uint32 _requiredComponents)
{
    requiredComponents = _requiredComponents;
}

inline uint32 SceneSystem::GetRequiredComponents() const
{
    return requiredComponents;
}

};
#endif //__DAVAENGINE_SCENE3D_SCENESYSTEM_H__
