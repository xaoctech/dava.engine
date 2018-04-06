#pragma once

#include "_gl.h"

namespace rhi
{
    
#define RHI_ENABLE_OPENGL_STATE_CACHE 1
    
#if (RHI_ENABLE_OPENGL_STATE_CACHE)
#define RHI_COMPARE_OPENGL_STATE(a, b) do { if ((a) == (b)) return; } while (0)
#else
#define RHI_COMPARE_OPENGL_STATE(a, b) do {} while (0)
#endif

struct OpenGLState
{
    enum Condition : int32
    {
        Undefined = -1,
        Disabled = 0,
        Enabled = 1,
    };

    enum State : GLenum
    {
        StateScissorTest,
        StateCullFace,
        StateDepthTest,
        StatePolygonOffsetFill,
        StateStencilTest,
        StateBlend,
        StateDepthWrite,

        StateCount
    };

    enum BufferBinding : GLenum
    {
        ArrayBuffer,
        ElementArrayBuffer,

        BufferBindingCount,
    };

    enum TextureTarget : GLenum
    {
        Texture2D,
        TextureCube,

        TextureTargetCount,
    };

    enum CullFace : uint32
    {
        Back,
        Front,

        CullFaceCount,
        CullFaceUndefined
    };

    enum FrontFace : uint32
    {
        Clockwise,
        CounterClockwise,

        FrontFaceCount,
        FrontFaceUndefined,
    };

    enum : uint32
    {
        MaxVertexAttributesCount = 32,
    };

    void SetEnabled(State state, Condition cond)
    {
        DVASSERT(state < StateCount);
        RHI_COMPARE_OPENGL_STATE(currentState[state], cond);

        currentState[state] = cond;
        if (cond == Condition::Undefined)
            return;

        if (state == StateDepthWrite)
        {
            GL_CALL(glDepthMask(cond == Condition::Enabled ? GL_TRUE : GL_FALSE));
        }
        else
        {
            switch (cond)
            {
            case Condition::Enabled:
                GL_CALL(glEnable(stateToOpenGLValues[state]));
                break;

            case Condition::Disabled:
                GL_CALL(glDisable(stateToOpenGLValues[state]));
                break;

            default:
                break;
            }
        }
    }

    void SetVertexAttribDivisor(uint32 idx, uint32 divisor)
    {
        DVASSERT(idx < MaxVertexAttributesCount);
        RHI_COMPARE_OPENGL_STATE(vertexAttribDivisorState[idx], divisor);

        vertexAttribDivisorState[idx] = divisor;
        
    #if defined(__DAVAENGINE_IPHONE__)
        GL_CALL(glVertexAttribDivisorEXT(idx, divisor));
    #elif defined(__DAVAENGINE_ANDROID__)
        if (glVertexAttribDivisor)
        {
            GL_CALL(glVertexAttribDivisor(idx, divisor));
        }
    #elif defined(__DAVAENGINE_MACOS__)
        GL_CALL(glVertexAttribDivisorARB(idx, divisor));
    #else
        GL_CALL(glVertexAttribDivisor(idx, divisor));
    #endif
    }

    void SetVertexAttributeEnabled(uint32 index, Condition condition)
    {
        DVASSERT(index < 32);
        RHI_COMPARE_OPENGL_STATE(vertexAttributes[index], condition);

        vertexAttributes[index] = condition;
        switch (condition)
        {
        case Condition::Enabled:
            GL_CALL(glEnableVertexAttribArray(index));
            break;

        case Condition::Disabled:
            GL_CALL(glDisableVertexAttribArray(index));
            break;

        default:
            break;
        }
    }

    void SetBufferBinding(BufferBinding binding, uint32 buffer)
    {
        DVASSERT(binding < BufferBindingCount);
        RHI_COMPARE_OPENGL_STATE(bufferBindings[binding], buffer);

        bufferBindings[binding] = buffer;
        GL_CALL(glBindBuffer(bufferBindingToOpenGLValues[binding], buffer));
    }

    void BindBuffer(GLenum binding, uint32 buffer)
    {
        switch (binding)
        {
        case GL_ARRAY_BUFFER:
            SetBufferBinding(BufferBinding::ArrayBuffer, buffer);
            break;

        case GL_ELEMENT_ARRAY_BUFFER:
            SetBufferBinding(BufferBinding::ElementArrayBuffer, buffer);
            break;

        default:
            DVASSERT(!"Invalid or unsupported buffer binding");
        }
    }

    void SetActiveTexture(uint32 index)
    {
        RHI_COMPARE_OPENGL_STATE(activeTexture, index);

        activeTexture = index;

        if (activeTexture != uint32(-1))
        {
            GL_CALL(glActiveTexture(GL_TEXTURE0 + index));
        }
    }

    void BindTextureToTarget(TextureTarget target, GLenum value)
    {
        DVASSERT(activeTexture != uint32(-1));
        RHI_COMPARE_OPENGL_STATE(textureBindings[target][activeTexture], value);

        textureBindings[target][activeTexture] = value;
        GL_CALL(glBindTexture(textureTargetToOpenGLValues[target], value));
    }

    void BindTexture(GLenum target, GLenum value)
    {
        switch (target)
        {
        case GL_TEXTURE_2D:
            BindTextureToTarget(TextureTarget::Texture2D, value);
            break;

        case GL_TEXTURE_CUBE_MAP:
            BindTextureToTarget(TextureTarget::TextureCube, value);
            break;

        default:
            DVASSERT(!"Invalid or unsupported texture target");
        }
    }

    void SetCullFace(CullFace cf)
    {
        RHI_COMPARE_OPENGL_STATE(cullFace, cf);

        cullFace = cf;
        GL_CALL(glCullFace(cullFaceToOpenGLValues[cf]));
    }

    void SetFrontFace(FrontFace ff)
    {
        RHI_COMPARE_OPENGL_STATE(frontFace, ff);

        frontFace = ff;
        GL_CALL(glFrontFace(frontFaceToOpenGLValues[ff]));
    }

    void UseProgram(uint32 prog)
    {
        RHI_COMPARE_OPENGL_STATE(activeProgram, prog);

        activeProgram = prog;
        GL_CALL(glUseProgram(prog));
    }

    void SetColorMask(bool r, bool g, bool b, bool a)
    {
        uint32 newMask = uint32(r) | (uint32(g) << 8) | (uint32(b) << 16) | (uint32(a) << 24);
        RHI_COMPARE_OPENGL_STATE(colorMask, newMask);

        colorMask = newMask;
        GL_CALL(glColorMask(r, g, b, a));
    }

    void SetDepthFunction(uint32 func)
    {
        RHI_COMPARE_OPENGL_STATE(depthFunction, func);

        depthFunction = func;
        if (func != uint32(-1))
        {
            GL_CALL(glDepthFunc(func));
        }
    }

    void SetViewport(int32 x, int32 y, int32 w, int32 h)
    {
    #if (RHI_ENABLE_OPENGL_STATE_CACHE)
        if ((x == viewport[0]) && (y == viewport[1]) && (w == viewport[2]) && (h == viewport[3]))
            return;
    #endif

        viewport[0] = x;
        viewport[1] = y;
        viewport[2] = w;
        viewport[3] = h;
        GL_CALL(glViewport(x, y, w, h));
    }

    OpenGLState()
    {
        InvalidateState();
    }

    void InvalidateState()
    {
        colorMask = uint32(-1);
        activeTexture = uint32(-1);
        activeProgram = uint32(-1);
        depthFunction = uint32(-1);
        cullFace = CullFaceUndefined;
        frontFace = FrontFaceUndefined;
        std::fill(std::begin(viewport), std::end(viewport), -1);
        std::fill(std::begin(currentState), std::end(currentState), Condition::Undefined);
        std::fill(std::begin(vertexAttribDivisorState), std::end(vertexAttribDivisorState), uint32(-1));
        std::fill(std::begin(bufferBindings), std::end(bufferBindings), uint32(-1));
        std::fill(std::begin(vertexAttributes), std::end(vertexAttributes), Condition::Undefined);
        for (auto& b : textureBindings)
        {
            std::fill(std::begin(b), std::end(b), uint32(-1));
        }
    }

private:
    Condition currentState[StateCount]{};
    uint32 vertexAttribDivisorState[MaxVertexAttributesCount]{};
    uint32 bufferBindings[BufferBindingCount]{};
    uint32 activeTexture = uint32(-1);
    uint32 activeProgram = uint32(-1);
    uint32 colorMask = uint32(-1);
    CullFace cullFace = CullFaceUndefined;
    FrontFace frontFace = FrontFaceUndefined;
    Condition vertexAttributes[32]{};
    uint32 depthFunction = uint32(-1);
    int32 viewport[4] = {};

    using TextureUnitBindings = GLuint[32];
    TextureUnitBindings textureBindings[TextureTargetCount] = {};

private:
    GLenum stateToOpenGLValues[StateCount] = { GL_SCISSOR_TEST, GL_CULL_FACE, GL_DEPTH_TEST, GL_POLYGON_OFFSET_FILL, GL_STENCIL_TEST, GL_BLEND };
    GLenum bufferBindingToOpenGLValues[BufferBindingCount] = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER };
    GLenum textureTargetToOpenGLValues[TextureTargetCount] = { GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP };
    GLenum cullFaceToOpenGLValues[CullFaceCount] = { GL_BACK, GL_FRONT };
    GLenum frontFaceToOpenGLValues[FrontFaceCount] = { GL_CW, GL_CCW };
};

extern OpenGLState glState;
}
