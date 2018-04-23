#include "rhi_ProgGLES2.h"
#include "../Common/rhi_Private.h"
#include "../Common/dbg_StatSet.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_RingBuffer.h"

#include "Logger/Logger.h"
using DAVA::Logger;

#include "rhi_GLES2.h"
#include "_gl.h"
#include "rhi_OpenGLState.h"

#include <stdio.h>
#include <string.h>

namespace rhi
{
//==============================================================================

typedef ResourcePool<ProgGLES2::ConstBuf, RESOURCE_CONST_BUFFER, ProgGLES2::ConstBuf::Desc, false> ConstBufGLES2Pool;
RHI_IMPL_POOL_SIZE(ProgGLES2::ConstBuf, RESOURCE_CONST_BUFFER, ProgGLES2::ConstBuf::Desc, false, 12 * 1024);

static RingBuffer _GLES2_DefaultConstRingBuffer;
uint32 ProgGLES2::ConstBuf::CurFrame = 0;

//==============================================================================

static void DumpShaderTextGLES2(const char* code, uint32 code_sz)
{
    char ss[64 * 1024];
    uint32 line_cnt = 0;

    if (code_sz < sizeof(ss))
    {
        strcpy(ss, code);

        const char* line = ss;
        for (char* s = ss; *s; ++s)
        {
            if (*s == '\r')
                *s = ' ';

            if (*s == '\n')
            {
                *s = 0;
                Logger::Info("%4u |  %s", 1 + line_cnt, line);
                line = s + 1;
                ++line_cnt;
            }
        }
    }
    else
    {
        Logger::Info(code);
    }
}

ProgGLES2::ProgGLES2(ProgType t)
    : type(t)
{
}

bool ProgGLES2::Construct(const char* srcCode)
{
    bool success = false;
    int stype = (type == PROG_VERTEX) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    GLCommand cmd1 = { GLCommand::CREATE_SHADER, { uint64(stype) } };

    ExecGL(&cmd1, 1);

    if (cmd1.retval)
    {
        uint32 s = cmd1.retval;
        int status = 0;
        char info[1024] = "";
        GLCommand cmd2[] =
        {
          { GLCommand::SHADER_SOURCE, { s, 1, uint64(&srcCode), 0 } },
          { GLCommand::COMPILE_SHADER, { s } },
          { GLCommand::GET_SHADER_IV, { s, GL_COMPILE_STATUS, uint64(&status) } },
          { GLCommand::GET_SHADER_INFO_LOG, { s, countof(info), 0, uint64(info) } }
        };

        ExecGL(cmd2, countof(cmd2));

        if (status)
        {
            shader = s;
            success = true;
        }
        else
        {
            Logger::Error("%sprog-compile failed:", (type == PROG_VERTEX) ? "v" : "f");
            Logger::Error(info);
            DumpShaderTextGLES2(srcCode, static_cast<unsigned>(strlen(srcCode)));
        }

        memset(cbufLastBoundData, 0, sizeof(cbufLastBoundData));
    }

    return success;
}

void ProgGLES2::Destroy()
{
}

void ProgGLES2::GetProgParams(uint32 progUid)
{
    for (uint32 i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
    {
        cbuf[i].location = DAVA::InvalidIndex;
        cbuf[i].count = 0;
    }

    GLint cnt = 0;
    GLCommand cmd1 = { GLCommand::GET_PROGRAM_IV, { progUid, GL_ACTIVE_UNIFORMS, uint64(&cnt) } };

    ExecGL(&cmd1, 1);

    for (uint32 u = 0; u != cnt; ++u)
    {
        char name[64];
        GLsizei length;
        GLint size;
        GLenum utype;
        GLCommand cmd2 = { GLCommand::GET_ACTIVE_UNIFORM, { progUid, u, uint64(sizeof(name) - 1), uint64(&length), uint64(&size), uint64(&utype), uint64(name) } };

        ExecGL(&cmd2, 1);

        for (uint32 i = 0; i != MAX_CONST_BUFFER_COUNT; ++i)
        {
            char n[16], n2[16];
            sprintf(n, "%s_Buffer%u[0]", (type == PROG_VERTEX) ? "VP" : "FP", i);
            sprintf(n2, "%s_Buffer%u", (type == PROG_VERTEX) ? "VP" : "FP", i);

            if (!strcmp(name, n) || !strcmp(name, n2))
            {
                int loc;
                GLCommand cmd3 = { GLCommand::GET_UNIFORM_LOCATION, { progUid, uint64(name) } };

                ExecGL(&cmd3, 1);
                loc = cmd3.retval;

                if (loc != -1)
                {
                    cbuf[i].location = loc;
                    cbuf[i].count = size;
                    break;
                }
            }
        }
    }

    // get texture location
    {
        char tname[countof(texunitLoc)][32];
        GLCommand cmd[countof(texunitLoc)];

        for (uint32 i = 0; i != countof(texunitLoc); ++i)
        {
            if (type == PROG_FRAGMENT)
                Snprintf(tname[i], countof(tname[i]), "FragmentTexture%u", i);
            else if (type == PROG_VERTEX)
                Snprintf(tname[i], countof(tname[i]), "VertexTexture%u", i);

            cmd[i].func = GLCommand::GET_UNIFORM_LOCATION;
            cmd[i].arg[0] = progUid;
            cmd[i].arg[1] = uint64(tname[i]);
        }

        ExecGL(cmd, countof(cmd));
        texunitCount = 0;

        for (uint32 i = 0; i != countof(texunitLoc); ++i)
        {
            int loc = cmd[i].retval;

            texunitLoc[i] = (loc != -1) ? loc : DAVA::InvalidIndex;

            if (loc != -1)
                ++texunitCount;
        }
    }

    prog = progUid;
    texunitInited = true;
}

uint32 ProgGLES2::SamplerCount()
{
    return texunitCount;
}

uint32 ProgGLES2::ConstBufferCount()
{
    return countof(cbuf);
}

Handle ProgGLES2::InstanceConstBuffer(uint32 bufIndex)
{
    Handle handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
    // DVASSERT(prog != 0);

    if ((prog != 0) && (bufIndex < countof(cbuf)) && (cbuf[bufIndex].location != DAVA::InvalidIndex))
    {
        handle = ConstBufGLES2Pool::Alloc();

        ConstBuf* cb = ConstBufGLES2Pool::Get(handle);

        if (cb->Construct(prog, cbufLastBoundData + bufIndex, cbuf[bufIndex].location, cbuf[bufIndex].count) == false)
        {
            ConstBufGLES2Pool::Free(handle);
            handle = InvalidHandle;
        }
    }

    return handle;
}

void ProgGLES2::SetupTextureUnits(uint32 baseUnit, GLCommand* commands, uint32& commandsCount)
{
    for (uint32 i = 0; i != countof(texunitLoc); ++i)
    {
        if (texunitLoc[i] != -1)
        {
            commands[commandsCount] = { GLCommand::SET_UNIFORM_1I, { texunitLoc[i], baseUnit + i } };
            ++commandsCount;
        }
    }
}

bool ProgGLES2::ConstBuf::Construct(uint32 prog, float** lastBoundData, uint32 loc, uint32 cnt)
{
    dataSize = cnt * 4 * sizeof(float);

    glProg = prog;
    location = loc;
    count = cnt;
    inst = nullptr;
    frame = 0;
    data = reinterpret_cast<float*>(::calloc(1, dataSize));
    lastInst = lastBoundData;
    *lastInst = nullptr;

    return true;
}

void ProgGLES2::ConstBuf::Destroy()
{
    if (data)
    {
        ::free(data);
        data = nullptr;
    }

    lastInst = nullptr;
    inst = nullptr;
    location = -1;
    count = 0;
}

uint32 ProgGLES2::ConstBuf::ConstCount()
{
    return count;
}

void ProgGLES2::ConstBuf::ReallocIfneeded()
{
    inst = nullptr;
}

bool ProgGLES2::ConstBuf::SetConst(uint32 const_i, uint32 const_count, const float* cdata)
{
    bool success = false;

    float* d = data + const_i * 4;
    float* d_end = (data + const_i * 4 + const_count * 4);
    float* end = data + count * 4;

    // this is workaround against too clever GLSL-compilers (Tegra),
    // when actual cbuf-array size is smaller that declared due to unused last elements
    if (d_end >= end)
        d_end = end;

    if (d < d_end)
    {
        memcpy(d, cdata, (d_end - d) * sizeof(float));
        ReallocIfneeded();
        success = true;
    }

    return success;
}

bool ProgGLES2::ConstBuf::SetConst(uint32 const_i, uint32 const_sub_i, const float* cdata, uint32 data_count)
{
    bool success = false;

    if (const_i < count && const_sub_i < 4)
    {
        memcpy(data + const_i * 4 + const_sub_i, cdata, data_count * sizeof(float));
        ReallocIfneeded();
        success = true;
    }

    return success;
}

const void* ProgGLES2::ConstBuf::Instance()
{
    if (frame != CurFrame)
    {
        inst = nullptr;
        *lastInst = nullptr;
    }

    if (inst == nullptr)
    {
        inst = _GLES2_DefaultConstRingBuffer.Alloc(count * 4);
        memcpy(inst, data, dataSize);
        frame = CurFrame;
    }

    return inst;
}

void ProgGLES2::ConstBuf::SetToRHI(uint32 progUid, const float* instData)
{
    DVASSERT(progUid == glProg);
 
#if (RHI_ENABLE_OPENGL_STATE_CACHE)
    if (instData != (*lastInst))
#endif
    {
        GL_CALL(glUniform4fv(location, count, instData));
        *lastInst = const_cast<float*>(instData);
    }
}

void ProgGLES2::ConstBuf::AdvanceFrame()
{
    ++CurFrame;
}

uint32 ProgGLES2::ShaderUid()
{
    return shader;
}

void ProgGLES2::InvalidateAllConstBufferInstances()
{
    ConstBuf::AdvanceFrame();
    _GLES2_DefaultConstRingBuffer.Reset();
}

static bool gles2_ConstBuffer_SetConst(Handle cb, uint32 const_i, uint32 const_count, const float* data)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);
    return self->SetConst(const_i, const_count, data);
}

static bool gles2_ConstBuffer_SetConst1(Handle cb, uint32 const_i, uint32 const_sub_i, const float* data, uint32 dataCount)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);
    return self->SetConst(const_i, const_sub_i, data, dataCount);
}

static void gles2_ConstBuffer_Delete(Handle cb)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);

    self->Destroy();
    ConstBufGLES2Pool::Free(cb);
}

namespace ConstBufferGLES2
{
void Init(uint32 maxCount)
{
    ConstBufGLES2Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_ConstBuffer_SetConst = &gles2_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv = &gles2_ConstBuffer_SetConst1;
    dispatch->impl_ConstBuffer_Delete = &gles2_ConstBuffer_Delete;
}

void InitializeRingBuffer(uint32 size)
{
    _GLES2_DefaultConstRingBuffer.Initialize(size);
}

void SetToRHI(const Handle cb, uint32 progUid, const float* instData)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);
    self->SetToRHI(progUid, instData);
}

const void* Instance(Handle cb)
{
    ProgGLES2::ConstBuf* self = ConstBufGLES2Pool::Get(cb);
    return self->Instance();
}
}

} // namespace rhi
