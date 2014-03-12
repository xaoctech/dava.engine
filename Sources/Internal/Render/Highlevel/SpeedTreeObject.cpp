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


#include "Render/Highlevel/SpeedTreeObject.h"
#include "Utils/Utils.h"

namespace DAVA 
{

SpeedTreeObject::SpeedTreeObject() :
    isAnimationEnabled(false)
{
}
    
SpeedTreeObject::~SpeedTreeObject()
{
}
    
void SpeedTreeObject::RecalcBoundingBox()
{
    bbox = AABBox3();

    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = renderBatchArray[k].renderBatch;
        PolygonGroup * pg = rb->GetPolygonGroup();
        if(pg)
        {
            if((pg->GetFormat() & EVF_TANGENT) > 0) //speedtree leaf batch
                bbox.AddAABBox(CalcBBoxForSpeedTreeLeafGeometry(pg));
            else
                bbox.AddAABBox(rb->GetBoundingBox());
        }   
    }
}
    
void SpeedTreeObject::SetTreeAnimationParams(const Vector3 & trunkOscillationParams, const Vector2 & leafOscillationParams)
{
    uint32 matCount = materials.size();
    for(uint32 i = 0; i < matCount; ++i)
    {
        NMaterial * material = materials[i];
        material->SetPropertyValue(FastName("trunkOscillationParams"), Shader::UT_FLOAT_VEC3, 1, &trunkOscillationParams);
        material->SetPropertyValue(FastName("leafOscillationParams"), Shader::UT_FLOAT_VEC2, 1, &leafOscillationParams);
    }
}
    
void SpeedTreeObject::SetAnimationEnabled(bool isEnabled)
{
    if(isAnimationEnabled != isEnabled)
    {
        isAnimationEnabled = isEnabled;
        
        NMaterial::eFlagValue flagValue = NMaterial::FlagOff;
        if(isAnimationEnabled) flagValue = NMaterial::FlagOn;
        
        uint32 matCount = materials.size();
        for(uint32 i = 0; i < matCount; ++i)
            materials[i]->SetFlag(FastName("WIND_ANIMATION"), flagValue);
    }
}
    
void SpeedTreeObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Mesh::Save(archive, serializationContext);
}
    
void SpeedTreeObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    Mesh::Load(archive, serializationContext);

    float32 treeHeight = bbox.GetSize().z;

    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = renderBatchArray[k].renderBatch;
        PolygonGroup * pg = rb->GetPolygonGroup();
        if(pg)
        {
            int32 vertexFormat = pg->GetFormat();
            
            bool isLeaf = ((vertexFormat & EVF_TANGENT) > 0); //speedtree leaf batch
            int32 vxCount = pg->GetVertexCount();
            int32 indCount = pg->GetIndexCount();
            PolygonGroup * newPG = new PolygonGroup();

			if(isLeaf)
				newPG->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_TANGENT | EVF_BINORMAL, vxCount, indCount);
			else
				newPG->AllocateData(EVF_VERTEX | EVF_COLOR | EVF_TEXCOORD0 | EVF_BINORMAL, vxCount, indCount);
            
            //copy indicies
            for(int32 i = 0; i < indCount; ++i)
            {
                int32 index;
                pg->GetIndex(i, index);
                newPG->SetIndex(i, index);
            }
            
            //copy vertex data
            for(int32 i = 0; i < vxCount; ++i)
            {
                Vector3 vxPosition;
                uint32 color;
                Vector2 vxTx;
                
                pg->GetCoord(i, vxPosition);
                pg->GetColor(i, color);
                pg->GetTexcoord(0, i, vxTx);
                
                newPG->SetCoord(i, vxPosition);
                newPG->SetColor(i, color);
                newPG->SetTexcoord(0, i, vxTx);
                
                float32 t0 = 0.f;
                if(isLeaf)
                {
                    Vector3 vxTangent;
                    pg->GetTangent(i, vxTangent);
                    newPG->SetTangent(i, vxTangent);
                    
                    t0 = vxPosition.Length() * 100;
                }
                
                float32 x = vxPosition.z / treeHeight;
                float32 flexebility = logf((expf(1.0) - 1) * x + 1);
                
                //Binormal: x: cos(T0);  y: sin(T0); z - flexebility
                Vector3 binormal(cosf(t0) * (.5f + x/2), sinf(t0) * (.5f + x/2), flexebility);
                
                newPG->SetBinormal(i, binormal);
            }
            
            rb->SetPolygonGroup(newPG);
        }
    }
    
    CollectMaterials();
}
    
RenderObject * SpeedTreeObject::Clone(RenderObject *newObject)
{
    if(!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<SpeedTreeObject>(this), "Can clone only SpeedTreeObject");
        newObject = new SpeedTreeObject();
    }

    Mesh::Clone(newObject);
    
    ((SpeedTreeObject *)newObject)->CollectMaterials();
    
    return newObject;
}
    
void SpeedTreeObject::CollectMaterials()
{
    materials.clear();

    Set<NMaterial *> materialSet;
    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = renderBatchArray[k].renderBatch;
        materialSet.insert(rb->GetMaterial());
    }

    Set<NMaterial *>::const_iterator endIt = materialSet.end();
    for(Set<NMaterial *>::const_iterator it = materialSet.begin(); it != endIt; ++it)
        materials.push_back(*it);
}
    
AABBox3 SpeedTreeObject::CalcBBoxForSpeedTreeLeafGeometry(PolygonGroup * pg)
{
    AABBox3 pgBbox;
    if(pg)
    {
        DVASSERT((pg->GetFormat() & EVF_TANGENT) > 0); //non speedtree leaf batch

        int32 vertexCount = pg->GetVertexCount();
        for(int32 vi = 0; vi < vertexCount; vi++)
        {
            Vector3 pivot;
            pg->GetTangent(vi, pivot);

            Vector3 pointX, pointY, pointZ;
            Vector3 offsetX, offsetY, offsetZ;

            pg->GetCoord(vi, pointZ);
            offsetX = offsetY = offsetZ = pointZ - pivot;

            Swap(offsetX.x, offsetX.z);
            Swap(offsetX.y, offsetX.z);
            
            pointX = pivot + offsetX;
            pointY = pivot + offsetY;

            pgBbox.AddPoint(pointX);
            pgBbox.AddPoint(pointY);
            pgBbox.AddPoint(pointZ);
        }
    }

    return pgBbox;
}

};
