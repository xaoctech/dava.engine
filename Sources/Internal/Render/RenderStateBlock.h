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
#ifndef __DAVAENGINE_RENDERSTATEBLOCK_H__
#define __DAVAENGINE_RENDERSTATEBLOCK_H__

#include "Render/RenderBase.h"
#include "Core/Core.h"

namespace DAVA
{

/*
class BlendEnable
{
public:
    inline void UpdateVarGL(bool value)
    {
#if defined(__DAVAENGINE_OPENGL__)
        if (value)glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
#elif defined(__DAVAENGINE_DIRECTX9__)
        
#endif
    }
    
};

class RenderStateBlock;
    
class RenderStateVariable
{
public:
    RenderStateVariable(RenderStateBlock * mainBlock);
    virtual ~RenderStateVariable();
    
    
    
};

template<class IMPL_CLASS, class V>
class RenderStateVariableImpl : public RenderStateVariable
{
public:
    RenderStateVariableImpl(RenderStateBlock * mainBlock);
    
    inline void UpdateVarGL() { renderStateImpl.UpdateVarGL(value); };
    inline void Set(const V & _value) { value = _value; };
                    
    V value;
    IMPL_CLASS renderStateImpl;
};


class RenderStateBlock
{
public:
    enum 
    {
        STATE_TYPE_UINT32 = 0,    
        STATE_TYPE_COLOR = 1,
    };
	enum
	{
        STATE_BLEND     = 0,
		STATE_BLENDFUNC = 1,  
        //        STATE_CULLFACE  = 1 << 2,     
        //        STATE_DEPTHTEST = 1 << 3,  

        STATE_ENABLE_TEXTURING = 2,    // Fixed func only
        STATE_COLOR = 3,  
	};
    enum
    {
        STATE_COUNT = 4
    };
    
    void Init(Core::eRenderer renderer);
    void Flush();
    
    
    typedef void (RenderStateBlock::*UpdateVarFunc)(uint32 value, uint32 oldValue);
    typedef void (RenderStateBlock::*UpdateVarFuncColor)(Color & value, Color & oldValue);

    void UpdateBlendGL(uint32 value, uint32 oldValue);
    void UpdateBlendDX(uint32 value, uint32 oldValue);
    
    void UpdateBlendFuncGL(uint32 func, uint32 oldValue);
    void UpdateBlendFuncDX(uint32 func, uint32 oldValue);

    void UpdateEnableTexturingGL(uint32 func, uint32 oldValue);
    void UpdateEnableTexturingDX(uint32 func, uint32 oldValue);
    
    void UpdateEnableColorGL(Color & value, Color & oldValue);
    void UpdateEnableColorDX(Color & value, Color & oldValue);

//    void UpdateCullFaceGL(uint32 value, uint32 oldValue);
//    void UpdateCullFaceDX(uint32 value, uint32 oldValue);

    void RegisterStateUInt32(uint32 state, UpdateVarFunc func);  
    void RegisterStateColor(uint32 state, UpdateVarFuncColor func);  
    
    
    
    inline void SetStateValue(uint32 state, uint32 value);
    inline uint32 GetStateValue(uint32 state); 
    
    
    
    uint32 activeStateSet;
    uint32 currentStateSet;
    
    uint32 stateType[STATE_COUNT];
    uint32 varValue[STATE_COUNT];
    Color varValueColor[STATE_COUNT];
    uint32 currentRendererValue[STATE_COUNT];
    Color currentRendererValueColor[STATE_COUNT];
    UpdateVarFunc varUpdateFuncArray[STATE_COUNT];
    UpdateVarFuncColor varUpdateFuncArrayColor[STATE_COUNT];
    uint32 varCount;
    
    
    bool isDebug;
};
    
    
inline void RenderStateBlock::SetStateValue(uint32 state, uint32 value)
{
    varValue[state] = value;
    currentStateSet |= (1 << state); // flag that value was changed
}

inline uint32 RenderStateBlock::GetStateValue(uint32 state) 
{
    return varValue[state];
}
    
*/
class Texture;
class Shader;
    
class RenderStateBlock
{
public:  
    enum 
    {
        // bit flags
        STATE_BLEND = 1 << 0,            // 
        STATE_DEPTH_TEST = 1 << 1,       
        STATE_DEPTH_WRITE = 1 << 2,
        STATE_STENCIL_TEST = 1 << 3,     
        STATE_CULL = 1 << 4,

        STATE_ALPHA_TEST = 1 << 5,          // fixed func only / in programmable pipeline can check for consistency
        STATE_TEXTURE0 = 1 << 8,            // fixed func only / in programmable pipeline only checks for consistency
        STATE_TEXTURE1 = 1 << 9,            // fixed func only / in programmable pipeline only checks for consistency
        STATE_TEXTURE2 = 1 << 10,            // fixed func only / in programmable pipeline only checks for consistency
        STATE_TEXTURE3 = 1 << 11,            // fixed func only / in programmable pipeline only checks for consistency
        
        STATE_COLORMASK_RED =  1 << 12,
        STATE_COLORMASK_GREEN = 1 << 13,
        STATE_COLORMASK_BLUE = 1 << 14,
        STATE_COLORMASK_ALPHA = 1 << 15,
        STATE_COLORMASK_ALL = (STATE_COLORMASK_RED | STATE_COLORMASK_GREEN | STATE_COLORMASK_BLUE | STATE_COLORMASK_ALPHA),
        
        // 4 bits for sourceBlendFactor
        // 4 bits for destBlendFactor
        // 
        // 32 bits * 4 for textures
        // 32 bits * 4 for color ??? can be switched to 4ub.
    };

    static const uint32 DEFAULT_2D_STATE = (STATE_TEXTURE0 | STATE_COLORMASK_ALL);
    static const uint32 DEFAULT_2D_STATE_BLEND = (STATE_BLEND | STATE_TEXTURE0 | STATE_COLORMASK_ALL);
    
    static const uint32 DEFAULT_3D_STATE = (STATE_DEPTH_WRITE | STATE_DEPTH_TEST | STATE_CULL | STATE_TEXTURE0 | STATE_COLORMASK_ALL);
    static const uint32 DEFAULT_3D_STATE_BLEND = (STATE_BLEND | STATE_DEPTH_WRITE | STATE_DEPTH_TEST | STATE_CULL | STATE_TEXTURE0 | STATE_COLORMASK_ALL);

    enum
    {
        STATE_CHANGED_COLOR = 1 << 0,       // bit marks that color was changed during set
        STATE_CHANGED_SRC_BLEND = 1 << 1,  
        STATE_CHANGED_DEST_BLEND = 1 << 2, 
        STATE_CHANGED_CULLMODE = 1 << 3,
        STATE_CHANGED_SHADER = 1 << 4,
        STATE_CHANGED_ALPHA_FUNC = 1 << 5,
    
        STATE_CHANGED_TEXTURE0 = 1 << 8,        
        STATE_CHANGED_TEXTURE1 = 1 << 9,        
        STATE_CHANGED_TEXTURE2 = 1 << 10,        
        STATE_CHANGED_TEXTURE3 = 1 << 11,

		STATE_CHANGED_STENCIL_REF = 1 << 12,
		STATE_CHANGED_STENCIL_MASK = 1 << 13,
		STATE_CHANGED_STENCIL_FUNC = 1 << 14,
		STATE_CHANGED_STENCIL_PASS = 1 << 15,
		STATE_CHANGED_STENCIL_FAIL = 1 << 16,
		STATE_CHANGED_STENCIL_ZFAIL = 1 << 17,
    };
    /*
        algorithm: 
        diffstate = state ^ oldstate;
        if (diffstate != 0)
        {
            go state by state
            // blend 
            // depth test
            // depth write
            // stencil
        }
        if (changevars != 0)
        {
            // go var by var
            
        }
     */
    
    /*enum
    {
        STATE_COLOR                 = 1 << 0, 
        STATE_BLEND_ENABLED         = 1 << 1,
        STATE_SRC_BLEND             = 1 << 2,
        STATE_DEST_BLEND            = 1 << 3, 
        STATE_BLEND_FUNC            = (1 << 2) | (1 << 3),
        STATE_TEXTURE_ENABLED       = 1 << 4,
        STATE_TEXTURE               = 1 << 5,
        STATE_DEPTH_TEST_ENABLED    = 1 << 6,
        STATE_DEPTH_WRITE_ENABLED   = 1 << 7,
        STATE_STENCIL_TEST_ENABLED  = 1 << 8,
//        STATE_ALPHA_TEST_ENABLED    = 1 << 9,
//        STATE_SCISSOR_TEST_ENABLED  = 1 << 10,
        
        
        STATE_TEXTURE_0
        
    };*/

    RenderStateBlock(Core::eRenderer _renderer);
    ~RenderStateBlock();
        
    Core::eRenderer renderer;
    uint32 state;
    uint32 changeSet;
    
    Color color;
    eBlendMode sourceFactor, destFactor;
    eFace cullMode;
    eCmpFunc alphaFunc;
    uint8    alphaFuncCmpValue;

	struct StencilState
	{
		StencilState();
		int32 ref;
		uint32 mask;
		eCmpFunc func[2];
		eStencilOp pass[2];
		eStencilOp fail[2];
		eStencilOp zFail[2];
	};
	StencilState stencilState;
    
    static const uint32 MAX_TEXTURE_LEVELS = 4;
    Texture * currentTexture[MAX_TEXTURE_LEVELS];
    Shader * shader;
    
    // STATE_COLOR
    inline void SetColor(float32 _r, float32 _g, float32 _b, float32 _a);
    inline void SetColor(const Color & _color);
	inline void ResetColor();
    inline const Color & GetColor() const;
    
    // STATE_BLEND_ENABLED
    inline void SetEnableBlendingInHW();
    
    // STATE_BLEND_FUNC
    inline void SetBlendMode(eBlendMode _sourceFactor, eBlendMode _destFactor);
	inline eBlendMode GetSrcBlend();
	inline eBlendMode GetDestBlend();
    
    // STATE_TEXTURE
    inline void SetTexture(Texture *texture, uint32 textureLevel = 0);
	inline Texture * GetTexture(uint32 textureLevel = 0);
    
    // SHADER
    inline void SetShader(Shader * shader);
    
    // CULL MODE
    inline void SetCullMode(eFace mode);
    
    // ALPHA
    inline void SetAlphaFunc(eCmpFunc func, float32 cmpValue);

	// STENCIL
	inline void SetStencilRef(int32 ref);
	inline void SetStencilMask(uint32 mask);
	inline void SetStencilFunc(eFace face, eCmpFunc func);
	inline void SetStencilPass(eFace face, eStencilOp operation);
	inline void SetStencilFail(eFace face, eStencilOp operation);
	inline void SetStencilZFail(eFace face, eStencilOp operation);


    
    
    inline void SetTextureLevelInHW(uint32 textureLevel);
    inline void SetBlendModeInHW();
    inline void SetDepthTestInHW();
    inline void SetDepthWriteInHW();
    inline void SetCullInHW();
    inline void SetCullModeInHW();
    inline void SetColorInHW();
    inline void SetColorMaskInHW();
	inline void SetStensilTestInHW();

    inline void SetAlphaTestInHW();
    inline void SetAlphaTestFuncInHW();

	inline void SetStencilRefInHW();
	inline void SetStencilMaskInHW();
	inline void SetStencilFuncInHW();
	inline void SetStencilPassInHW();
	inline void SetStencilFailInHW();
	inline void SetStencilZFailInHW();
	inline void SetStencilOpInHW();
    
    /**
        Function to reset state to original zero state.
     */
    void Reset(bool doHardwareReset);
    
    /**
        Function to flush state into graphics hardware
        It checks what was changed from previous flush
        It updates previous state block to current state
     */
    void Flush(RenderStateBlock * previousState);
    
    /**
        Compare states
     */
    bool IsEqual(RenderStateBlock * anotherState);


#if defined(__DAVAENGINE_DIRECTX9__)
	static IDirect3DDevice9 * direct3DDevice; 
#endif
};

// Implementation of inline functions
inline void RenderStateBlock::SetColor(float32 _r, float32 _g, float32 _b, float32 _a)
{
    if ((color.r != _r) || (color.g != _g) || (color.b != _b) || (color.a != _a))
    {
        color.r = _r;
        color.g = _g;
        color.b = _b;
        color.a = _a;
        changeSet |= STATE_CHANGED_COLOR;
    }
}

inline void RenderStateBlock::SetColor(const Color & _color)
{
    if (color != _color)
    {
        color = _color;
        changeSet |= STATE_CHANGED_COLOR;
    }
}

inline void RenderStateBlock::ResetColor()
{
    if ((color.r != 1.0f) || (color.g != 1.0f) || (color.b != 1.0f) || (color.a != 1.0f))
    {
        color.r = color.g = color.b = color.a = 1.0f;
        changeSet |= STATE_CHANGED_COLOR;
    }
}


inline const Color & RenderStateBlock::GetColor() const
{
    return color;
}

inline void RenderStateBlock::SetBlendMode(eBlendMode _sourceFactor, eBlendMode _destFactor)
{
    if ((sourceFactor != _sourceFactor) || (destFactor != _destFactor))
    {       
        sourceFactor = _sourceFactor;
        destFactor = _destFactor;
        changeSet |= STATE_CHANGED_SRC_BLEND | STATE_CHANGED_DEST_BLEND;
    }
}
    
// SHADER
inline void RenderStateBlock::SetShader(Shader * _shader)
{
    shader = _shader;
// Rethink concept of caching shader / shader data
//    if (shader != _shader)
//    {
//        shader = _shader;
//        changeSet |= STATE_CHANGED_SHADER;
//    }
}

// CULL MODE
inline void RenderStateBlock::SetCullMode(eFace _cullMode)
{
    if (cullMode != _cullMode)
    {   
        cullMode = _cullMode;
        changeSet |= STATE_CHANGED_CULLMODE;
    }
}

inline void RenderStateBlock::SetAlphaFunc(eCmpFunc func, float32 cmpValue)
{
    uint8 newCmpValue = (uint8)(cmpValue * 255.0f);
    if ((alphaFunc != func) || (alphaFuncCmpValue != newCmpValue))
    {
        alphaFunc = func;
        alphaFuncCmpValue = newCmpValue;
        changeSet |= STATE_CHANGED_ALPHA_FUNC;
    }
}
    
inline eBlendMode RenderStateBlock::GetSrcBlend()
{
    return sourceFactor;
}

inline eBlendMode RenderStateBlock::GetDestBlend()
{
    return destFactor;
}

// STATE_TEXTURE
inline void RenderStateBlock::SetTexture(Texture *texture, uint32 textureLevel)
{
    if (currentTexture[textureLevel] != texture)
    {
        currentTexture[textureLevel] = texture;
        changeSet |= (STATE_CHANGED_TEXTURE0 << textureLevel);
    }
}

inline Texture * RenderStateBlock::GetTexture(uint32 textureLevel)
{
    return currentTexture[textureLevel];
}

inline void RenderStateBlock::SetStencilRef(int32 ref)
{
	if(stencilState.ref != ref)
	{
		stencilState.ref = ref;
		changeSet |= STATE_CHANGED_STENCIL_REF;
	}
}

inline void RenderStateBlock::SetStencilMask(uint32 mask)
{
	if(stencilState.mask != mask)
	{
		stencilState.mask = mask;
		changeSet |= STATE_CHANGED_STENCIL_MASK;
	}
}

inline void RenderStateBlock::SetStencilFunc(eFace face, eCmpFunc func)
{
	if(face == FACE_FRONT || face == FACE_BACK)
	{
		if(stencilState.func[face] != func)
		{
			stencilState.func[face] = func;
			changeSet |= STATE_CHANGED_STENCIL_FUNC;
		}
	}
	else
	{
		if((stencilState.func[0] != func) || (stencilState.func[1] != func))
		{
			stencilState.func[0] = stencilState.func[1] = func;
			changeSet |= STATE_CHANGED_STENCIL_FUNC;
		}
	}
}

inline void RenderStateBlock::SetStencilPass(eFace face, eStencilOp operation)
{
	if(face == FACE_FRONT || face == FACE_BACK)
	{
		if(stencilState.pass[face] != operation)
		{
			stencilState.pass[face] = operation;
			changeSet |= STATE_CHANGED_STENCIL_PASS;
		}
	}
	else
	{
		if((stencilState.pass[0] != operation) || (stencilState.pass[1] != operation))
		{
			stencilState.pass[0] = stencilState.pass[1] = operation;
			changeSet |= STATE_CHANGED_STENCIL_PASS;
		}
	}
}

inline void RenderStateBlock::SetStencilFail(eFace face, eStencilOp operation)
{
	if(face == FACE_FRONT || face == FACE_BACK)
	{
		if(stencilState.fail[face] != operation)
		{
			stencilState.fail[face] = operation;
			changeSet |= STATE_CHANGED_STENCIL_FAIL;
		}
	}
	else
	{
		if((stencilState.fail[0] != operation) || (stencilState.fail[1] != operation))
		{
			stencilState.fail[0] = stencilState.fail[1] = operation;
			changeSet |= STATE_CHANGED_STENCIL_FAIL;
		}
	}
}

inline void RenderStateBlock::SetStencilZFail(eFace face, eStencilOp operation)
{
	if(face == FACE_FRONT || face == FACE_BACK)
	{
		if(stencilState.zFail[face] != operation)
		{
			stencilState.zFail[face] = operation;
			changeSet |= STATE_CHANGED_STENCIL_ZFAIL;
		}
	}
	else
	{
		if((stencilState.zFail[0] != operation) || (stencilState.zFail[1] != operation))
		{
			stencilState.zFail[0] = stencilState.zFail[1] = operation;
			changeSet |= STATE_CHANGED_STENCIL_ZFAIL;
		}
	}
}
    
};

#endif // __DAVAENGINE_RENDERSTATEBLOCK_H__
