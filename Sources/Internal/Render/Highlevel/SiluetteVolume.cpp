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
=====================================================================================*/

#include "Render/Highlevel/SiluetteVolume.h"
#include "Render/RenderManager.h"
#include "FileSystem/FilePath.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Shader.h"

namespace DAVA
{

REGISTER_CLASS(SiluetteVolume);

SiluetteVolume::SiluetteVolume()
:   siluettePolygonGroup(0)
,   uniformSiluetteScale(-1)
,   uniformSiluetteColor(-1)
{
	shader = new Shader();
	shader->LoadFromYaml("~res:/Shaders/Siluette/siluette.shader");
	shader->Recompile();

    aabbox = AABBox3(Vector3(), Vector3());

    uniformSiluetteScale = shader->FindUniformLocationByName("siluetteScale");
    DVASSERT(uniformSiluetteScale != -1);

    uniformSiluetteColor = shader->FindUniformLocationByName("siluetteColor");
    DVASSERT(uniformSiluetteColor != -1);

    uniformSiluetteExponent = shader->FindUniformLocationByName("siluetteExponent");
    DVASSERT(uniformSiluetteExponent != -1);
}

SiluetteVolume::~SiluetteVolume()
{
	SafeRelease(shader);
	SafeRelease(siluettePolygonGroup);
}

void SiluetteVolume::Draw(Camera * camera)
{
    if(!renderObject)return;
    Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
    if (!worldTransformPtr)
    {
        return;
    }

    Matrix4 finalMatrix = (*worldTransformPtr) * camera->GetMatrix();
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

    Matrix4 projMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION);

    RenderManager::Instance()->RemoveState(RenderState::STATE_CULL);
    RenderManager::Instance()->RemoveState(RenderState::STATE_DEPTH_TEST);
    RenderManager::Instance()->RemoveState(RenderState::STATE_BLEND);
    RenderManager::Instance()->AppendState(RenderState::STATE_STENCIL_TEST);

    RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_NOTEQUAL);
    RenderManager::State()->SetStencilRef(64);
    RenderManager::State()->SetStencilMask(0x000000F0);

    RenderManager::State()->SetStencilPass(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
    RenderManager::State()->SetStencilFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
    RenderManager::State()->SetStencilZFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);

	RenderManager::Instance()->SetShader(shader);
	RenderManager::Instance()->SetRenderData(siluettePolygonGroup->renderDataObject);

	RenderManager::Instance()->FlushState();
    RenderManager::Instance()->AttachRenderData();

    shader->SetUniformValue(uniformSiluetteScale, siluetteScale);
    shader->SetUniformValue(uniformSiluetteExponent, siluetteExponent);
    shader->SetUniformColor4(uniformSiluetteColor, siluetteColor);

	if (siluettePolygonGroup->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, siluettePolygonGroup->indexCount, EIF_16, 0);
	}
	else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, siluettePolygonGroup->indexCount, EIF_16, siluettePolygonGroup->indexArray);
	}

    RenderManager::Instance()->AppendState(RenderState::STATE_CULL);
    RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
    RenderManager::Instance()->AppendState(RenderState::STATE_DEPTH_TEST);
}

void SiluetteVolume::SetSiluetteExponent(float32 value)
{
    siluetteExponent = value;
}

void SiluetteVolume::SetSiluetteScale(float32 scale)
{
    siluetteScale = scale;
}

void SiluetteVolume::SetSiluetteColor(const Color & c)
{
    siluetteColor = c;
}

void SiluetteVolume::SetSiluettePolygonGroup(PolygonGroup * polygonGroup)
{
    SafeRelease(siluettePolygonGroup);
    siluettePolygonGroup = SafeRetain(polygonGroup);
}
};
