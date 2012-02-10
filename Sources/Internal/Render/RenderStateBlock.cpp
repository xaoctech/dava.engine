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
#include "Render/RenderStateBlock.h"
#include "Render/RenderManager.h"
#include "Render/Shader.h"
#include "Platform/Thread.h"


namespace DAVA
{

#if defined(__DAVAENGINE_DIRECTX9__)
IDirect3DDevice9 * RenderStateBlock::direct3DDevice = 0; 
#endif

RenderStateBlock::RenderStateBlock(Core::eRenderer _renderer)
    : renderer(_renderer)
{
	Reset(false);
}

RenderStateBlock::~RenderStateBlock()
{
    
}
    
    
/**
    Function to reset state to original zero state.
 */
void RenderStateBlock::Reset(bool doHardwareReset)
{
    state = 0;
    changeSet = 0;
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 1.0f;
    sourceFactor = BLEND_ONE;
    destFactor = BLEND_ZERO;
    for (uint32 idx = 0; idx < MAX_TEXTURE_LEVELS; ++idx)
        currentTexture[idx] = 0;
    alphaFunc = CMP_ALWAYS;
    shader = 0;
    cullMode = FACE_BACK;
    
    if (doHardwareReset)
    {
        SetColorInHW();
        SetEnableBlendingInHW();
        SetBlendModeInHW();
        SetDepthTestInHW();
        SetDepthWriteInHW();
        SetColorMaskInHW();
        
        if (renderer != Core::RENDERER_OPENGL_ES_2_0)
        {
            SetAlphaTestInHW();
            SetAlphaTestFuncInHW();
        }
        
        for (uint32 textureLevel = 0; textureLevel < MAX_TEXTURE_LEVELS; ++textureLevel)
        {
            SetTextureLevelInHW(textureLevel);
        }
    }
}
bool RenderStateBlock::IsEqual(RenderStateBlock * anotherState)
{
    if (state != anotherState->state)
        return false;
    
    // check texture first for early rejection 
    if (currentTexture[0] != anotherState->currentTexture[0])return false;

    if (state & STATE_BLEND)
    {
        if (destFactor != anotherState->destFactor)return false;
        if (sourceFactor != anotherState->sourceFactor)return false;
    }
    
    if (color != anotherState->color)return false;
    
    if (currentTexture[1] != anotherState->currentTexture[1])return false;
    if (currentTexture[2] != anotherState->currentTexture[2])return false;
    if (currentTexture[3] != anotherState->currentTexture[3])return false;
    
    
    
    
    return true;
}

void RenderStateBlock::Flush(RenderStateBlock * previousState)
{
    uint32 diffState = state ^ previousState->state;
    if (diffState != 0)
    {
        if (diffState & STATE_BLEND)
            SetEnableBlendingInHW();
        
        if (diffState & STATE_DEPTH_TEST)
            SetDepthTestInHW();
        
        if (diffState & STATE_DEPTH_WRITE)
            SetDepthWriteInHW();
        
        if (diffState & STATE_CULL)
            SetCullInHW();
        
        if (diffState & STATE_COLORMASK_ALL)
            SetColorMaskInHW();

		if (diffState & STATE_STENCIL_TEST)
			SetStensilTestInHW();
        
        if (renderer != Core::RENDERER_OPENGL_ES_2_0)
            if (diffState & STATE_ALPHA_TEST)
                SetAlphaTestInHW();
        
        changeSet |= diffState & (STATE_TEXTURE0 | STATE_TEXTURE1 | STATE_TEXTURE2 | STATE_TEXTURE3);
        
        previousState->state = state;
    }
    
    if (changeSet != 0)
    {
        if (changeSet & STATE_CHANGED_COLOR)
            if (color != previousState->color)
            {
                SetColorInHW();
                previousState->color = color;
            }
        if (changeSet & (STATE_CHANGED_SRC_BLEND | STATE_CHANGED_DEST_BLEND))
            if (sourceFactor != previousState->sourceFactor || destFactor != previousState->destFactor)
            {
                SetBlendModeInHW();
                previousState->sourceFactor = sourceFactor;
                previousState->destFactor = destFactor;
            }
        
        if (changeSet & STATE_CHANGED_CULLMODE)
            if (cullMode != previousState->cullMode)
            {
                SetCullModeInHW();
                previousState->cullMode = cullMode;
            }
        
        if (renderer != Core::RENDERER_OPENGL_ES_2_0)
            if (changeSet & STATE_CHANGED_ALPHA_FUNC)
                if ((alphaFunc != previousState->alphaFunc) || (alphaFuncCmpValue != previousState->alphaFuncCmpValue))
                {
                    SetAlphaTestFuncInHW();
                    previousState->alphaFunc = alphaFunc;
                    previousState->alphaFuncCmpValue = alphaFuncCmpValue;
                }
        
        if (changeSet & STATE_CHANGED_TEXTURE0)
        {
            SetTextureLevelInHW(0);
            previousState->currentTexture[0] = currentTexture[0];
        }
        if (changeSet & STATE_CHANGED_TEXTURE1)
        {
            SetTextureLevelInHW(1);
            previousState->currentTexture[1] = currentTexture[1];
        }
        if (changeSet & STATE_CHANGED_TEXTURE2)
        {
            SetTextureLevelInHW(2);
            previousState->currentTexture[2] = currentTexture[2];
        }
        if (changeSet & STATE_CHANGED_TEXTURE3)
        {
            SetTextureLevelInHW(3);
            previousState->currentTexture[3] = currentTexture[3];
        }

		if (changeSet & STATE_CHANGED_STENCIL_REF)
		{
			SetStencilRefInHW();
			previousState->stencilState.ref = stencilState.ref;
		}
		if (changeSet & STATE_CHANGED_STENCIL_MASK)
		{
			SetStencilMaskInHW();
			previousState->stencilState.mask = stencilState.mask;
		}
		if (changeSet & STATE_CHANGED_STENCIL_FUNC)
		{
			SetStencilFuncInHW();
			previousState->stencilState.func[0] = stencilState.func[0];
			previousState->stencilState.func[1] = stencilState.func[1];
		}

		if (changeSet & STATE_CHANGED_STENCIL_PASS)
		{
			SetStencilPassInHW();
			previousState->stencilState.pass[0] = stencilState.pass[0];
			previousState->stencilState.pass[1] = stencilState.pass[1];
		}
		if (changeSet & STATE_CHANGED_STENCIL_FAIL)
		{
			SetStencilFailInHW();
			previousState->stencilState.fail[0] = stencilState.fail[0];
			previousState->stencilState.fail[1] = stencilState.fail[1];
		}
		if (changeSet & STATE_CHANGED_STENCIL_ZFAIL)
		{
			SetStencilZFailInHW();
			previousState->stencilState.zFail[0] = stencilState.zFail[0];
			previousState->stencilState.zFail[1] = stencilState.zFail[1];
		}
		if((changeSet & STATE_CHANGED_STENCIL_PASS) || (changeSet & STATE_CHANGED_STENCIL_FAIL) || (changeSet & STATE_CHANGED_STENCIL_ZFAIL))
		{
			SetStencilOpInHW();
			previousState->stencilState.pass[0] = stencilState.pass[0];
			previousState->stencilState.pass[1] = stencilState.pass[1];
			previousState->stencilState.fail[0] = stencilState.fail[0];
			previousState->stencilState.fail[1] = stencilState.fail[1];
			previousState->stencilState.zFail[0] = stencilState.zFail[0];
			previousState->stencilState.zFail[1] = stencilState.zFail[1];
		}

#if defined(__DAVAENGINE_OPENGL__)
        RENDER_VERIFY(glActiveTexture(GL_TEXTURE0));
#endif
//        if (changeSet & STATE_CHANGED_SHADER)
//        {
//            if (shader != previousState->shader)
//            {
//            }
//        }

        changeSet = 0;
        previousState->changeSet = 0;
    }
    if (shader)shader->Bind();
    else Shader::Unbind();
    previousState->shader = shader;

}
    
    
#if defined (__DAVAENGINE_OPENGL__)
inline void RenderStateBlock::SetColorInHW()
{
    if (renderer != Core::RENDERER_OPENGL_ES_2_0)
        RENDER_VERIFY(glColor4f(color.r * color.a, color.g * color.a, color.b * color.a, color.a));
}
    
inline void RenderStateBlock::SetColorMaskInHW()
{
    GLboolean redMask = (state & STATE_COLORMASK_RED) != 0;
    GLboolean greenMask = (state & STATE_COLORMASK_GREEN) != 0;
    GLboolean blueMask = (state & STATE_COLORMASK_BLUE) != 0;
    GLboolean alphaMask = (state & STATE_COLORMASK_ALPHA) != 0;
    
    RENDER_VERIFY(glColorMask(redMask, 
                              greenMask, 
                              blueMask, 
                              alphaMask));
}

inline void RenderStateBlock::SetStensilTestInHW()
{
	if (state & STATE_STENCIL_TEST)
	{
		RENDER_VERIFY(glEnable(GL_STENCIL_TEST));
	}
	else
	{
		RENDER_VERIFY(glDisable(GL_STENCIL_TEST));
	}
}

inline void RenderStateBlock::SetEnableBlendingInHW()
{
    if (state & STATE_BLEND)
    {
        RENDER_VERIFY(glEnable(GL_BLEND));
    }
    else 
    {
        RENDER_VERIFY(glDisable(GL_BLEND));
    }
}
    
inline void RenderStateBlock::SetCullInHW()
{
    if (state & STATE_CULL)
    {
        RENDER_VERIFY(glEnable(GL_CULL_FACE));
    }
    else 
    {
        RENDER_VERIFY(glDisable(GL_CULL_FACE));
    }
}

inline void RenderStateBlock::SetCullModeInHW()
{
    RENDER_VERIFY(glCullFace(CULL_FACE_MAP[cullMode]));
}


inline void RenderStateBlock::SetBlendModeInHW()
{
    RENDER_VERIFY(glBlendFunc(BLEND_MODE_MAP[sourceFactor], BLEND_MODE_MAP[destFactor]));
}

inline void RenderStateBlock::SetTextureLevelInHW(uint32 textureLevel)
{
    if(currentTexture[textureLevel])
    {
        RENDER_VERIFY(glActiveTexture(GL_TEXTURE0 + textureLevel));
        
        if (state & (STATE_TEXTURE0 << textureLevel))
            glEnable(GL_TEXTURE_2D);
        else 
            glDisable(GL_TEXTURE_2D);
        
        RENDER_VERIFY(glBindTexture(GL_TEXTURE_2D, currentTexture[textureLevel]->id));
    }else
    {
        RENDER_VERIFY(glActiveTexture(GL_TEXTURE0 + textureLevel));

        if (state & (STATE_TEXTURE0 << textureLevel))
            glEnable(GL_TEXTURE_2D);
        else 
            glDisable(GL_TEXTURE_2D);
        
        RENDER_VERIFY(glBindTexture(GL_TEXTURE_2D, 0));
    }    
}
inline void RenderStateBlock::SetDepthTestInHW()
{
    if(state & STATE_DEPTH_TEST)
    {
        RENDER_VERIFY(glEnable(GL_DEPTH_TEST));
    }
    else
    {
        RENDER_VERIFY(glDisable(GL_DEPTH_TEST));
    }    
}

inline void RenderStateBlock::SetDepthWriteInHW()
{
    if(state & STATE_DEPTH_WRITE)
    {
        RENDER_VERIFY(glDepthMask(GL_TRUE));
    }
    else
    {
        RENDER_VERIFY(glDepthMask(GL_FALSE));
    }
}
    
inline void RenderStateBlock::SetAlphaTestInHW()
{
    if(state & STATE_ALPHA_TEST)
    {
        RENDER_VERIFY(glEnable(GL_ALPHA_TEST));
    }
    else
    {
        RENDER_VERIFY(glDisable(GL_ALPHA_TEST));
    }
}

inline void RenderStateBlock::SetAlphaTestFuncInHW()
{
     RENDER_VERIFY(glAlphaFunc(COMPARE_FUNCTION_MAP[alphaFunc], alphaFuncCmpValue) );
}

inline void RenderStateBlock::SetStencilRefInHW()
{
	changeSet |= STATE_CHANGED_STENCIL_FUNC;
}

inline void RenderStateBlock::SetStencilMaskInHW()
{
	changeSet |= STATE_CHANGED_STENCIL_FUNC;
}

inline void RenderStateBlock::SetStencilFuncInHW()
{
	if(stencilState.func[0] == (stencilState.func[1]))
	{
		RENDER_VERIFY(glStencilFunc(COMPARE_FUNCTION_MAP[stencilState.func[0]], stencilState.ref, stencilState.mask));
	}
	else
	{
		RENDER_VERIFY(glStencilFuncSeparate(CULL_FACE_MAP[FACE_FRONT], COMPARE_FUNCTION_MAP[stencilState.func[0]], stencilState.ref, stencilState.mask));
		RENDER_VERIFY(glStencilFuncSeparate(CULL_FACE_MAP[FACE_BACK], COMPARE_FUNCTION_MAP[stencilState.func[1]], stencilState.ref, stencilState.mask));
	}
}

inline void RenderStateBlock::SetStencilPassInHW()
{
	//nothing
}

inline void RenderStateBlock::SetStencilFailInHW()
{
	//nothing
}

inline void RenderStateBlock::SetStencilZFailInHW()
{
	//nothing
}

inline void RenderStateBlock::SetStencilOpInHW()
{
	RENDER_VERIFY(glStencilOpSeparate(CULL_FACE_MAP[FACE_FRONT], STENCIL_OP_MAP[stencilState.fail[0]], STENCIL_OP_MAP[stencilState.zFail[0]], STENCIL_OP_MAP[stencilState.pass[0]]));
	RENDER_VERIFY(glStencilOpSeparate(CULL_FACE_MAP[FACE_BACK], STENCIL_OP_MAP[stencilState.fail[1]], STENCIL_OP_MAP[stencilState.zFail[1]], STENCIL_OP_MAP[stencilState.pass[1]]));
}

    
#elif defined(__DAVAENGINE_DIRECTX9__)
    
inline void RenderStateBlock::SetColorInHW()
{
	//if (renderer != Core::RENDERER_OPENGL_ES_2_0)
	//	RENDER_VERIFY(glColor4f(color.r * color.a, color.g * color.a, color.b * color.a, color.a));
}
    
inline void RenderStateBlock::SetColorMaskInHW()
{
    DWORD flags = 0;
    if (state & STATE_COLORMASK_RED)
        flags |= D3DCOLORWRITEENABLE_RED;
    
    if (state & STATE_COLORMASK_GREEN)
        flags |= D3DCOLORWRITEENABLE_GREEN;
    
    if (state & STATE_COLORMASK_BLUE)
        flags |= D3DCOLORWRITEENABLE_BLUE;
    
    if (state & STATE_COLORMASK_ALPHA)
        flags |= D3DCOLORWRITEENABLE_ALPHA;
    
    RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, flags));
}

inline void RenderStateBlock::SetEnableBlendingInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, (state & STATE_BLEND) != 0));
}

inline void RenderStateBlock::SetCullInHW()
{
	if (!(state & STATE_CULL))
	{
		RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	}
}

inline void RenderStateBlock::SetCullModeInHW()
{
	if ((state & STATE_CULL))
	{
		RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CULLMODE, CULL_FACE_MAP[cullMode]));
	}
}


inline void RenderStateBlock::SetBlendModeInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_SRCBLEND, BLEND_MODE_MAP[sourceFactor]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_DESTBLEND, BLEND_MODE_MAP[destFactor]));
}

inline void RenderStateBlock::SetTextureLevelInHW(uint32 textureLevel)
{
	IDirect3DTexture9 * texture = 0;
	if(currentTexture[textureLevel])
		texture = currentTexture[textureLevel]->id;

	if (state & (STATE_TEXTURE0 << textureLevel))
	{
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE ));
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE ));
	}else 
	{
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 ));
		RENDER_VERIFY(direct3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2));
	}
	RENDER_VERIFY(direct3DDevice->SetTexture(textureLevel, texture));
}
inline void RenderStateBlock::SetDepthTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ZENABLE, (state & STATE_DEPTH_TEST) != 0));
}

inline void RenderStateBlock::SetDepthWriteInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ZWRITEENABLE , (state & STATE_DEPTH_WRITE) !=0));
}

inline void RenderStateBlock::SetAlphaTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, (state & STATE_ALPHA_TEST) != 0));
}

inline void RenderStateBlock::SetAlphaTestFuncInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHAFUNC, COMPARE_FUNCTION_MAP[alphaFunc]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_ALPHAREF , alphaFuncCmpValue));
}

inline void RenderStateBlock::SetStensilTestInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILENABLE, (state & STATE_STENCIL_TEST) != 0));
}

inline void RenderStateBlock::SetStencilRefInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILREF, stencilState.ref));
}

inline void RenderStateBlock::SetStencilMaskInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILMASK, stencilState.mask));
}

inline void RenderStateBlock::SetStencilFuncInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILFUNC, stencilState.func[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILFUNC, stencilState.func[1]));
}

inline void RenderStateBlock::SetStencilPassInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILPASS, stencilState.pass[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILPASS, stencilState.pass[1]));
}

inline void RenderStateBlock::SetStencilFailInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILFAIL, stencilState.fail[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILFAIL, stencilState.fail[1]));
}

inline void RenderStateBlock::SetStencilZFailInHW()
{
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_STENCILZFAIL, stencilState.zFail[0]));
	RENDER_VERIFY(direct3DDevice->SetRenderState(D3DRS_CCW_STENCILZFAIL, stencilState.zFail[1]));
}

inline void RenderStateBlock::SetStencilOpInHW()
{
}

#endif 
    
RenderStateBlock::StencilState::StencilState()
{
	ref = 0;
	mask = 0xFFFFFFFF;
	func[0] = func[1] = CMP_ALWAYS;
	pass[0] = pass[1] = STENCILOP_KEEP;
	fail[0] = fail[1] = STENCILOP_KEEP;
	zFail[0] = zFail[1] = STENCILOP_KEEP;
} 

};
