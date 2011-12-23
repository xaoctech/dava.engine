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
#include "Scene3D/SphereNode.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/3D/StaticMesh.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

namespace DAVA 
{
SphereNode::SphereNode(Scene * _scene)
:	MeshInstanceNode(_scene)
{
    sphereMesh = NULL;
    SetQuality(80);
}
	
SphereNode::~SphereNode()
{
	SafeRelease(sphereMesh);
}

void SphereNode::Update(float32 timeElapsed)
{
    MeshInstanceNode::Update(timeElapsed);
}
    
void SphereNode::Draw()
{
    MeshInstanceNode::Draw();
}

SceneNode* SphereNode::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
    {
        dstNode = new SphereNode(scene);
    }

    MeshInstanceNode::Clone(dstNode);
    
    ((SphereNode*)dstNode)->sphereMesh = (StaticMesh *)sphereMesh->Clone();

    return dstNode;
}

void SphereNode::CreateSphere(float32 radius, Color c)
{
    RGBColor color(c.r * 255, c.g * 255, c.b * 255, c.a * 255);
    
	SafeRelease(sphereMesh);
    sphereMesh = new StaticMesh(GetScene());
    sphereMesh->Create(1);
    
    //====== Общее количество треугольников 
    const int32 gnTria = (quality + 1) * quality * 2; 
    //====== Общее количество вершин 
    const int32 gnVert = (quality+1) * quality + 2; 
    
    PolygonGroup *sphere = sphereMesh->GetPolygonGroup(0);
	sphere->AllocateData( EVF_VERTEX | EVF_COLOR, gnVert, gnTria * 3, 0);

    //====== Формирование массива вершин 
    //====== Северный полюс 
    sphere->SetCoord(0, Vector3(0, radius, 0));
    sphere->SetColor(0, color);
    
    //====== Индекс последней вершины (на южном полюсе) 
    int32 last = gnVert - 1; //====== Южный полюс 

    sphere->SetCoord(last, Vector3(0, -radius, 0));
    sphere->SetColor(last, color);

    //====== Подготовка констант 
    float32 da = PI / (quality +2.f); 
    float32 db = 2.f * PI / quality;
    float32 af = PI - da/2.f; 
    float32 bf = 2.f * PI - db/2.f; 

    //=== Индекс вершины, следующей за северным полюсом 
    int32 n = 1; 
    
    //=== Цикл по широтам 
    for ( float32 a = da; a < af; a += da) 
    { 
        //=== Координата у постоянна для всего кольца
        float32 y = radius * cosf(a);
        
        //====== Вспомогательная точка 
        float32 xz = radius * sinf(a); 
        
        //====== Цикл по секциям (долгота) 
        for ( float32 b = 0.f; b < bf; n++, b += db)
        {
            // Координаты проекции в экваториальной плоскости
            float32 x = xz * sin(b);
            float32 z = xz * cos(b); 
    
            //====== Вершина, нормаль и цвет 
            sphere->SetCoord(n, Vector3(x, y, z));
            sphere->SetColor(n, color);
        }
    } 

    //====== Формирование массива индексов 
    //====== Треугольники вблизи полюсов 
    for (n = 0; n < quality; ++n)
    { 
        int32 n3 = n*3;
        //====== Индекс общей вершины (северный полюс) 
        sphere->SetIndex(n3 + 0, 0);
        //====== Индекс текущей вершины 
        sphere->SetIndex(n3 + 1, n + 1);
        //====== Замыкание 
        sphere->SetIndex(n3 + 2, n == quality - 1 ? 1 : n + 2);
        
        //====== Индекс общей вершины (южный полюс) 
        int32 nTri = 3*(gnTria-quality+n);
        sphere->SetIndex(nTri + 0, gnVert - 1);
        sphere->SetIndex(nTri + 1, gnVert - 2 - n);
        sphere->SetIndex(nTri + 2, gnVert - 2 - ( (1 + n) % quality));
    } 

    //====== Треугольники разбиения колец 
    //====== Вершина, следующая за полюсом 
    int k = 1; 
    //====== gnSects - номер следующего треугольника 
    n = quality; 
    for (int32 i = 0; i < quality; ++i, k += quality) 
    { 
        for (int32 j = 0; j < quality; ++j, n += 2) 
        { 
            int32 n3 = n*3;
            //======= Индекс общей вершины 
            float32 n0 = k + j;
            sphere->SetIndex(n3 + 0, n0);
            //======= Индекс текущей вершины 
            float32 n1 = k + quality + j;
            sphere->SetIndex(n3 + 1, n1);
            //======= Замыкание 
            float32 n2 = k + quality + ((j + 1) % quality);
            sphere->SetIndex(n3 + 2, n2);

            //======= To же для второго треугольника 
            sphere->SetIndex(n3 + 3 + 0, n0);
            sphere->SetIndex(n3 + 3 + 1, n2);
            sphere->SetIndex(n3 + 3 + 2, k + ((j + 1) % quality));
        }
    }            

    AddPolygonGroup(sphereMesh, 0, NULL);
}
    
void SphereNode::SetQuality(int32 newQuality)
{
    quality = newQuality;
}

};
