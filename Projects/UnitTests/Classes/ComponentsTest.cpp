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


ComponentsTest::ComponentsTest()
: TestTemplate<ComponentsTest>("ComponentsTest")
{
    RegisterFunction(this, &ComponentsTest::RegisterEntityTest, "RegisterEntityTest", NULL);
    RegisterFunction(this, &ComponentsTest::AddComponentTest, "AddComponentTest", NULL);
	RegisterFunction(this, &ComponentsTest::AddComponentTest2, "AddComponentTest2", NULL);
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
	Scene *scene = new Scene();

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
	Scene *scene = new Scene();

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
	Scene *scene = new Scene();

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


