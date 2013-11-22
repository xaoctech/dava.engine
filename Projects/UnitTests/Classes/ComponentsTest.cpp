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



#include "ComponentsTest.h"

template <class Type>
void RemoveFromVector(DAVA::Vector<Type *> elements, const Type * element)
{
    DAVA::uint32 size = elements.size();
    for(DAVA::uint32 index = 0; index < size; ++index)
    {
        if(element == elements[index])
        {
            elements[index] = elements[size - 1];
            elements.pop_back();
            return;
        }
    }
    
    DVASSERT(0);
}


SingleComponentSystem::SingleComponentSystem(Scene * scene)
    :   SceneSystem(scene)
{
}

void SingleComponentSystem::AddEntity(Entity * entity)
{
    entities.push_back(entity);
    
    for(int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint32 flags = 1 << id;
        if((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for(uint32 c = 0; c < componentsCount; ++c)
            {
                AddComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
    
}

void SingleComponentSystem::RemoveEntity(Entity * entity)
{
    for(int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint32 flags = 1 << id;
        if((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for(uint32 c = 0; c < componentsCount; ++c)
            {
                RemoveComponent(entity, entity->GetComponent(id, c));
            }
        }
    }

    
    
    RemoveFromVector(entities, entity);
}

void SingleComponentSystem::AddComponent(Entity * entity, Component * component)
{
    components.push_back(component);
}

void SingleComponentSystem::RemoveComponent(Entity * entity, Component * component)
{
    RemoveFromVector(components, component);
}


//=====
MultiComponentSystem::MultiComponentSystem(Scene * scene)
:   SceneSystem(scene)
{
}

void MultiComponentSystem::AddEntity(Entity * entity)
{
    entities.push_back(entity);
    
    for(int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint32 flags = 1 << id;
        if((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for(uint32 c = 0; c < componentsCount; ++c)
            {
                AddComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
    
}

void MultiComponentSystem::RemoveEntity(Entity * entity)
{
    for(int32 id = 0; id < Component::COMPONENT_COUNT; ++id)
    {
        uint32 flags = 1 << id;
        if((flags & GetRequiredComponents()) == flags)
        {
            uint32 componentsCount = entity->GetComponentCount(id);
            for(uint32 c = 0; c < componentsCount; ++c)
            {
                RemoveComponent(entity, entity->GetComponent(id, c));
            }
        }
    }
    
    RemoveFromVector(entities, entity);
}

void MultiComponentSystem::AddComponent(Entity * entity, Component * component)
{
    components[component->GetType()].push_back(component);
}

void MultiComponentSystem::RemoveComponent(Entity * entity, Component * component)
{
    RemoveFromVector(components[component->GetType()], component);
}







ComponentsTest::ComponentsTest()
: TestTemplate<ComponentsTest>("ComponentsTest")
{
    RegisterFunction(this, &ComponentsTest::RegisterEntityTest, "RegisterEntityTest", NULL);
    RegisterFunction(this, &ComponentsTest::AddComponentTest, "AddComponentTest", NULL);
	RegisterFunction(this, &ComponentsTest::AddComponentTest2, "AddComponentTest2", NULL);
    RegisterFunction(this, &ComponentsTest::MultiComponentTest1, "MultiComponentTest1", NULL);
	RegisterFunction(this, &ComponentsTest::MultiComponentTest2, "MultiComponentTest2", NULL);
}

void ComponentsTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
}


void ComponentsTest::UnloadResources()
{
    RemoveAllControls();
}

void ComponentsTest::RegisterEntityTest( PerfFuncData * data )
{
	Scene *scene = CreateSingleComponentScene();

	Entity *e1 = new Entity();
	e1->AddComponent(new LightComponent());
	e1->AddComponent(new ActionComponent());

	scene->AddNode(e1);
	scene->RemoveNode(e1);

	e1->Release();
	scene->Release();
}

void ComponentsTest::AddComponentTest( PerfFuncData * data )
{
	Scene *scene = CreateSingleComponentScene();

	Entity *e1 = new Entity();

	scene->AddNode(e1);

	e1->AddComponent(new ActionComponent());
	e1->AddComponent(new LightComponent());

	e1->RemoveComponent(Component::ACTION_COMPONENT);
	e1->RemoveComponent(Component::LIGHT_COMPONENT);

	scene->RemoveNode(e1);

	e1->Release();
	scene->Release();

}

void ComponentsTest::AddComponentTest2( PerfFuncData * data )
{
	Scene *scene = CreateSingleComponentScene();

	Entity *e1 = new Entity();

	scene->AddNode(e1);

	Component *a = new ActionComponent();
	Component *l = new LightComponent();

	e1->AddComponent(a);
	e1->AddComponent(l);

	e1->RemoveComponent(a);
	e1->RemoveComponent(l);

	a = l = NULL;

	scene->RemoveNode(e1);

	e1->Release();
	scene->Release();
}



void ComponentsTest::MultiComponentTest1( PerfFuncData * data )
{
	Scene *scene = CreateMultiComponentScene();
    
	Entity *e1 = new Entity();
    
	scene->AddNode(e1);
    
	Component *a = new ActionComponent();
	Component *l = new LightComponent();
    
	e1->AddComponent(a);
	e1->AddComponent(l);
    
	e1->RemoveComponent(a);
	e1->RemoveComponent(l);
    
	a = l = NULL;
    
	scene->RemoveNode(e1);
    
	e1->Release();
	scene->Release();
}

void ComponentsTest::MultiComponentTest2( PerfFuncData * data )
{
	Scene *scene = CreateMultiComponentScene();
    
	Entity *e1 = new Entity();
    
	scene->AddNode(e1);
    
	Component *a = new ActionComponent();
	Component *l = new LightComponent();
    
	e1->AddComponent(a);
	e1->AddComponent(l);
    
	e1->RemoveComponent(a);
	e1->RemoveComponent(l);
    
	a = l = NULL;
    
	scene->RemoveNode(e1);
    
	e1->Release();
	scene->Release();
}


Scene * ComponentsTest::CreateSingleComponentScene() const
{
	Scene *scene = new Scene();
    
    SingleComponentSystem * testSystemLight = new SingleComponentSystem(scene);
    SingleComponentSystem * testSystemAction = new SingleComponentSystem(scene);
    
    scene->AddSystem(testSystemLight, 1 << Component::LIGHT_COMPONENT);
    scene->AddSystem(testSystemAction, 1 << Component::ACTION_COMPONENT);

    return scene;
}

Scene * ComponentsTest::CreateMultiComponentScene() const
{
	Scene *scene = new Scene();
    
    MultiComponentSystem * testSystem = new MultiComponentSystem(scene);
    
    scene->AddSystem(testSystem, (1 << Component::LIGHT_COMPONENT) | (1 << Component::ACTION_COMPONENT));
    
    return scene;
}
