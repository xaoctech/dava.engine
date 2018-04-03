#include "../Common/rhi_Private.h"
#include "../rhi_Public.h"
#include "../rhi_ShaderCache.h"
#include "../Common/rhi_Pool.h"
#include "../Common/rhi_Utils.h"

#include "rhi_ProgGLES2.h"
#include "rhi_GLES2.h"

#include "Logger/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
using DAVA::Logger;

#include "_gl.h"
#include "rhi_OpenGLState.h"

#define SAVE_GLES_SHADERS 0

namespace rhi
{
namespace PipelineStateGLES2
{
static GLenum cachedBlendSrc = 0;
static GLenum cachedBlendDst = 0;
}

struct
VertexDeclGLES2
{
    struct
    vattr_t
    {
        const GLvoid* pointer;
        GLint size;
        GLenum type;
        int divisor;
        GLboolean normalized;
        // bool enabled;
    };

    static vattr_t vattr[VATTR_COUNT];

    VertexDeclGLES2()
        : elemCount(0)
        , streamCount(0)
        , vattrInited(false)
    {
        memset(stride, 0, sizeof(stride));
    }

    void Construct(const VertexLayout& layout)
    {
        elemCount = 0;

        for (uint32 i = 0; i != layout.ElementCount(); ++i)
        {
            if (layout.ElementSemantics(i) == VS_PAD)
                continue;

            uint32 stream_i = layout.ElementStreamIndex(i);

            switch (layout.ElementDataType(i))
            {
            case VDT_FLOAT:
            {
                elem[elemCount].type = GL_FLOAT;
                elem[elemCount].normalized = GL_FALSE;
            }
            break;

            case VDT_HALF:
            {
                elem[elemCount].type = GetHalfFloatEnumValue();
                elem[elemCount].normalized = GL_FALSE;
            }
            break;

            case VDT_INT16N:
            {
                elem[elemCount].type = GL_SHORT;
                elem[elemCount].normalized = GL_TRUE;
            }
            break;

            case VDT_INT16:
            {
                elem[elemCount].type = GL_SHORT;
                elem[elemCount].normalized = GL_FALSE;
            }
            break;

            case VDT_UINT8N:
            {
                elem[elemCount].type = GL_UNSIGNED_BYTE;
                elem[elemCount].normalized = GL_TRUE;
            }
            break;

            case VDT_INT8N:
            {
                elem[elemCount].type = GL_BYTE;
                elem[elemCount].normalized = GL_TRUE;
            }
            break;

            case VDT_UINT8:
            {
                elem[elemCount].type = GL_UNSIGNED_BYTE;
                elem[elemCount].normalized = GL_FALSE;
            }
            break;

            default:
            {
            }
            }

            switch (layout.ElementSemantics(i))
            {
            case VS_POSITION:
                sprintf(elem[elemCount].name, "attr_position%u", layout.ElementSemanticsIndex(i));
                break;
            case VS_NORMAL:
                sprintf(elem[elemCount].name, "attr_normal%u", layout.ElementSemanticsIndex(i));
                break;
            case VS_TEXCOORD:
                sprintf(elem[elemCount].name, "attr_texcoord%u", layout.ElementSemanticsIndex(i));
                break;
            case VS_COLOR:
                sprintf(elem[elemCount].name, "attr_color%u", layout.ElementSemanticsIndex(i));
                break;
            case VS_TANGENT:
                strcpy(elem[elemCount].name, "attr_tangent");
                break;
            case VS_BINORMAL:
                strcpy(elem[elemCount].name, "attr_binormal");
                break;
            case VS_BLENDWEIGHT:
                strcpy(elem[elemCount].name, "attr_blendweight");
                break;
            case VS_BLENDINDEX:
                strcpy(elem[elemCount].name, "attr_blendindex");
                break;

            default:
                strcpy(elem[elemCount].name, "<unsupported>");
            }

            elem[elemCount].count = layout.ElementDataCount(i);
            elem[elemCount].offset = reinterpret_cast<void*>(uint64(layout.ElementOffset(i)));
            elem[elemCount].index = DAVA::InvalidIndex;
            elem[elemCount].streamIndex = stream_i;
            elem[elemCount].attrDivisor = (layout.StreamFrequency(stream_i) == VDF_PER_INSTANCE) ? 1 : 0;

            stride[stream_i] = layout.Stride(stream_i);

            ++elemCount;
        }

        streamCount = layout.StreamCount();
        vattrInited = false;
    }
    void InitVattr(int gl_prog, bool forceExecute = false)
    {
        GLCommand cmd[16];

        for (uint32 i = 0; i != elemCount; ++i)
        {
            cmd[i].func = GLCommand::GET_ATTRIB_LOCATION;
            cmd[i].arg[0] = gl_prog;
            cmd[i].arg[1] = uint64_t(elem[i].name);
        }

        ExecGL(cmd, elemCount, forceExecute);

        for (uint32 i = 0; i != elemCount; ++i)
            elem[i].index = cmd[i].retval;

        vattrInited = true;
    }
    void SetToRHI(uint32 firstVertex, uint32 vertexStreamCount, const Handle* vb) const
    {
        DVASSERT(vattrInited);

        uint32 base[MAX_VERTEX_STREAM_COUNT];
        int attr_used[VATTR_COUNT];

        for (uint32 s = 0; s != streamCount; ++s)
            base[s] = firstVertex * stride[s];

        memset(attr_used, 0, sizeof(attr_used));

        static uint32 cur_stride[MAX_VERTEX_STREAM_COUNT];
        static uint32 cur_stream_count = 0;
        static bool needInit = true;

        if (needInit)
        {
            memset(vattr, 0, sizeof(vattr));
            memset(cur_stride, 0, sizeof(cur_stride));
            needInit = false;
        }

        //Trace("gl.vattr-array\n");
        //Trace("  base= %u  stride= %u  first_v= %u\n",base,stride,firstVertex);

        for (uint32 i = 0; i != elemCount; ++i)
        {
            uint32 idx = elem[i].index;

            if (idx != DAVA::InvalidIndex)
                attr_used[idx] = 1;
        }

        for (uint32 i = 0; i != countof(attr_used); ++i)
        {
            glState.SetVertexAttributeEnabled(i, attr_used[i] ? OpenGLState::Enabled : OpenGLState::Disabled);
        }

        uint32 stream = unsigned(-1);

        for (uint32 i = 0; i != elemCount; ++i)
        {
            if (!VAttrCacheValid || cur_stream_count != streamCount)
            {
                if (elem[i].streamIndex != stream)
                {
                    stream = elem[i].streamIndex;
                    DVASSERT(stream < streamCount);
                    VertexBufferGLES2::SetToRHI(vb[stream]);
                }
            }
            stream = elem[i].streamIndex;

            uint32 idx = elem[i].index;

            if (idx != DAVA::InvalidIndex)
            {
                glState.SetVertexAttributeEnabled(idx, OpenGLState::Enabled);

                if (!VAttrCacheValid || vattr[idx].size != elem[i].count || vattr[idx].type != elem[i].type || vattr[idx].normalized != GLboolean(elem[i].normalized) || cur_stride[stream] != stride[stream] || vattr[idx].pointer != static_cast<const GLvoid*>(base[stream] + static_cast<uint8_t*>(elem[i].offset)))
                {
                    GL_CALL(glVertexAttribPointer(idx, elem[i].count, elem[i].type, GLboolean(elem[i].normalized), stride[stream], static_cast<const GLvoid*>(base[stream] + static_cast<uint8_t*>(elem[i].offset))));

                    vattr[idx].size = elem[i].count;
                    vattr[idx].type = elem[i].type;
                    vattr[idx].normalized = GLboolean(elem[i].normalized);
                    vattr[idx].pointer = static_cast<const GLvoid*>(base[stream] + static_cast<uint8_t*>(elem[i].offset));
                }

                if (DeviceCaps().isInstancingSupported && (!VAttrCacheValid || vattr[idx].divisor != elem[i].attrDivisor))
                {
                    glState.SetVertexAttribDivisor(idx, elem[i].attrDivisor);
                    vattr[idx].divisor = elem[i].attrDivisor;
                }
            }

            cur_stride[stream] = stride[stream];
        }
        cur_stream_count = streamCount;

        VAttrCacheValid = true;
    }
    static void InvalidateVAttrCache()
    {
        VAttrCacheValid = false;
    }

    static void InvalidateVAttrCacheForTools()
    {
        InvalidateVAttrCache();
    }

    struct
    Elem
    {
        char name[32];
        uint32 index;

        uint32 type;
        uint32 count;
        uint32 streamIndex;
        int normalized;
        int attrDivisor;
        void* offset;
    };

    Elem elem[VATTR_COUNT]; //-V730_NOINIT
    uint32 elemCount;
    uint32 stride[MAX_VERTEX_STREAM_COUNT];
    uint32 streamCount;
    uint32 vattrInited : 1;

    static bool VAttrCacheValid;
};

bool VertexDeclGLES2::VAttrCacheValid = false;
VertexDeclGLES2::vattr_t VertexDeclGLES2::vattr[VATTR_COUNT];

class PipelineStateGLES2_t
{
public:
    PipelineStateGLES2_t() = default;

    struct VertexProgGLES2 : public ProgGLES2
    {
        VertexProgGLES2()
            : ProgGLES2(PROG_VERTEX)
        {
        }

        void SetToRHI(uint32 layoutUID, uint32 firstVertex, uint32 vertexStreamCount, const Handle* vb) const
        {
            if (layoutUID == VertexLayout::InvalidUID)
            {
                vdecl.SetToRHI(firstVertex, vertexStreamCount, vb);
            }
            else
            {
                for (std::vector<PipelineStateGLES2_t::VertexProgGLES2::vdecl_t>::iterator v = altVdecl.begin(), v_end = altVdecl.end(); v != v_end; ++v)
                {
                    if (v->layoutUID == layoutUID)
                    {
                        v->vdecl.SetToRHI(firstVertex, vertexStreamCount, vb);
                        break;
                    }
                }
            }
        }

        VertexDeclGLES2 vdecl;
        DAVA::FastName uid;

        struct vdecl_t
        {
            VertexDeclGLES2 vdecl;
            uint32 layoutUID;
        };

        mutable std::vector<vdecl_t> altVdecl;
    };

    struct FragmentProgGLES2 : public ProgGLES2
    {
        FragmentProgGLES2()
            : ProgGLES2(PROG_FRAGMENT)
        {
        }
        DAVA::FastName uid;
    };

    struct program_t
    {
        VertexProgGLES2* vprog = nullptr;
        FragmentProgGLES2* fprog = nullptr;
        uint32 glProg = 0;
    };

    struct ProgramEntry
    {
        uint32 vprogSrcHash;
        VertexProgGLES2* vprog;
        uint32 fprogSrcHash;
        FragmentProgGLES2* fprog;
        uint32 glProg;
        int refCount;
    };

    static bool AcquireProgram(const PipelineState::Descriptor& desc, program_t* prog);
    static void ReleaseProgram(const program_t& prog);

    program_t prog;

    GLenum blendSrc;
    GLenum blendDst;
    bool blendEnabled;

    GLboolean maskR;
    GLboolean maskG;
    GLboolean maskB;
    GLboolean maskA;

    static std::vector<ProgramEntry> _ProgramEntry;
};

typedef ResourcePool<PipelineStateGLES2_t, RESOURCE_PIPELINE_STATE, PipelineState::Descriptor, false> PipelineStateGLES2Pool;
RHI_IMPL_POOL_SIZE(PipelineStateGLES2_t, RESOURCE_PIPELINE_STATE, PipelineState::Descriptor, false, 16 * 1024);

std::vector<PipelineStateGLES2_t::ProgramEntry> PipelineStateGLES2_t::_ProgramEntry;

//------------------------------------------------------------------------------

bool PipelineStateGLES2_t::AcquireProgram(const PipelineState::Descriptor& desc, PipelineStateGLES2_t::program_t* prog)
{
    bool success = false;
    bool doAdd = true;
    uint32 vprogSrcHash;
    uint32 fprogSrcHash;
    const std::vector<uint8>& vprog_bin = rhi::ShaderCache::GetProg(desc.vprogUid);
    const std::vector<uint8>& fprog_bin = rhi::ShaderCache::GetProg(desc.fprogUid);

    vprogSrcHash = DAVA::HashValue_N(reinterpret_cast<const char*>(&vprog_bin[0]), static_cast<uint32>(strlen(reinterpret_cast<const char*>(&vprog_bin[0]))));
    fprogSrcHash = DAVA::HashValue_N(reinterpret_cast<const char*>(&fprog_bin[0]), static_cast<uint32>(strlen(reinterpret_cast<const char*>(&fprog_bin[0]))));

    for (std::vector<PipelineStateGLES2_t::ProgramEntry>::iterator p = _ProgramEntry.begin(), p_end = _ProgramEntry.end(); p != p_end; ++p)
    {
        if (p->vprogSrcHash == vprogSrcHash && p->fprogSrcHash == fprogSrcHash)
        {
            prog->vprog = p->vprog;
            prog->fprog = p->fprog;
            prog->glProg = p->glProg;
            doAdd = false;
            success = true;
            break;
        }
    }

#if SAVE_GLES_SHADERS

    static uint32 progIndex = 0;

    if (doAdd)
    {
        DAVA::FileSystem::Instance()->CreateDirectory("~doc:/ShaderSources");

        DAVA::File* vfile = DAVA::File::Create(DAVA::Format("~doc:/ShaderSources/vertex-prog-%03d.sl", progIndex), DAVA::File::CREATE | DAVA::File::WRITE);
        if (vfile)
        {
            vfile->Write("//", 2);
            vfile->WriteLine(desc.vprogUid.c_str());
            vfile->WriteLine("");
            vfile->Write((const char*)(&vprog_bin[0]), strlen((const char*)(&vprog_bin[0])));
            SafeRelease(vfile);
        }

        DAVA::File* ffile = DAVA::File::Create(DAVA::Format("~doc:/ShaderSources/fragment-prog-%03d.sl", progIndex), DAVA::File::CREATE | DAVA::File::WRITE);
        if (ffile)
        {
            ffile->Write("//", 2);
            ffile->WriteLine(desc.fprogUid.c_str());
            ffile->WriteLine("");
            ffile->Write((const char*)(&fprog_bin[0]), strlen((const char*)(&fprog_bin[0])));
            SafeRelease(ffile);
        }
    }

    progIndex++;

#endif

    if (doAdd)
    {
        ProgramEntry entry;
        bool vprog_valid = false;
        bool fprog_valid = false;

        // construct vprog

        entry.vprog = new VertexProgGLES2();
        if (entry.vprog->Construct(reinterpret_cast<const char*>(&vprog_bin[0])))
        {
            entry.vprog->vdecl.Construct(desc.vertexLayout);
            vprog_valid = true;
        }

        // construct fprog

        entry.fprog = new FragmentProgGLES2();
        if (entry.fprog->Construct(reinterpret_cast<const char*>(&fprog_bin[0])))
        {
            fprog_valid = true;
        }

        // construct pipeline-state

        if (vprog_valid && fprog_valid)
        {
            GLCommand cmd1[] =
            {
              { GLCommand::CREATE_PROGRAM, { 0 } },
            };

            ExecGL(cmd1, countof(cmd1));

            uint32 gl_prog = cmd1[0].retval;
            GLCommand cmd2[] =
            {
              { GLCommand::ATTACH_SHADER, { gl_prog, entry.vprog->ShaderUid() } },
              { GLCommand::ATTACH_SHADER, { gl_prog, entry.fprog->ShaderUid() } },
              { GLCommand::LINK_PROGRAM, { gl_prog } },
            };

            ExecGL(cmd2, countof(cmd2));

            if (cmd2[2].retval)
            {
                entry.vprog->vdecl.InitVattr(gl_prog);
                entry.vprog->GetProgParams(gl_prog);
                entry.fprog->GetProgParams(gl_prog);

                uint32 setupTextureUnintsCommandCount = 2; // space for push + use
                GLCommand setupTextureUnitsCommands[MAX_VERTEX_TEXTURE_SAMPLER_COUNT + MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT + 4] = {};

                entry.vprog->SetupTextureUnits(0, setupTextureUnitsCommands, setupTextureUnintsCommandCount);
                entry.fprog->SetupTextureUnits(entry.vprog->SamplerCount(), setupTextureUnitsCommands, setupTextureUnintsCommandCount);

                if (setupTextureUnintsCommandCount > 2)
                {
                    GLint currentProgram = 0;
                    setupTextureUnitsCommands[0] = { GLCommand::GET_CURRENT_PROGRAM_PTR, { uint64(&currentProgram) } };
                    setupTextureUnitsCommands[1] = { GLCommand::SET_CURRENT_PROGRAM_PTR, { uint64(&gl_prog) } };
                    setupTextureUnitsCommands[setupTextureUnintsCommandCount++] = { GLCommand::VALIDATE_PROGRAM, { gl_prog } };
                    setupTextureUnitsCommands[setupTextureUnintsCommandCount++] = { GLCommand::SET_CURRENT_PROGRAM_PTR, { uint64(&currentProgram) } };
                    ExecGL(setupTextureUnitsCommands, setupTextureUnintsCommandCount);
                }

                entry.vprog->uid = desc.vprogUid;
                entry.vprogSrcHash = vprogSrcHash;
                entry.fprog->uid = desc.fprogUid;
                entry.fprogSrcHash = fprogSrcHash;
                entry.glProg = gl_prog;

                success = true;
            }
        }

        _ProgramEntry.push_back(entry);
        prog->vprog = entry.vprog;
        prog->fprog = entry.fprog;
        prog->glProg = entry.glProg;
    }

    return success;
}

//------------------------------------------------------------------------------

void PipelineStateGLES2_t::ReleaseProgram(const PipelineStateGLES2_t::program_t& prog)
{
}

//------------------------------------------------------------------------------

static void gles2_PipelineState_Delete(Handle ps)
{
    PipelineStateGLES2Pool::Free(ps);
}

//------------------------------------------------------------------------------

static Handle gles2_PipelineState_Create(const PipelineState::Descriptor& desc)
{
    Handle handle = PipelineStateGLES2Pool::Alloc();
    ;
    PipelineStateGLES2_t* ps = PipelineStateGLES2Pool::Get(handle);
    const std::vector<uint8>& vprog_bin = rhi::ShaderCache::GetProg(desc.vprogUid);
    const std::vector<uint8>& fprog_bin = rhi::ShaderCache::GetProg(desc.fprogUid);

    if (PipelineStateGLES2_t::AcquireProgram(desc, &(ps->prog)))
    {
        ps->blendEnabled = desc.blending.rtBlend[0].blendEnabled;

        switch (desc.blending.rtBlend[0].colorSrc)
        {
        case BLENDOP_ZERO:
            ps->blendSrc = GL_ZERO;
            break;
        case BLENDOP_ONE:
            ps->blendSrc = GL_ONE;
            break;
        case BLENDOP_SRC_ALPHA:
            ps->blendSrc = GL_SRC_ALPHA;
            break;
        case BLENDOP_INV_SRC_ALPHA:
            ps->blendSrc = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case BLENDOP_SRC_COLOR:
            ps->blendSrc = GL_SRC_COLOR;
            break;
        case BLENDOP_DST_COLOR:
            ps->blendSrc = GL_DST_COLOR;
            break;
        }

        switch (desc.blending.rtBlend[0].colorDst)
        {
        case BLENDOP_ZERO:
            ps->blendDst = GL_ZERO;
            break;
        case BLENDOP_ONE:
            ps->blendDst = GL_ONE;
            break;
        case BLENDOP_SRC_ALPHA:
            ps->blendDst = GL_SRC_ALPHA;
            break;
        case BLENDOP_INV_SRC_ALPHA:
            ps->blendDst = GL_ONE_MINUS_SRC_ALPHA;
            break;
        case BLENDOP_SRC_COLOR:
            ps->blendDst = GL_SRC_COLOR;
            break;
        case BLENDOP_DST_COLOR:
            ps->blendDst = GL_DST_COLOR;
            break;
        }

        ps->maskR = desc.blending.rtBlend[0].writeMask & COLORMASK_R;
        ps->maskG = desc.blending.rtBlend[0].writeMask & COLORMASK_G;
        ps->maskB = desc.blending.rtBlend[0].writeMask & COLORMASK_B;
        ps->maskA = desc.blending.rtBlend[0].writeMask & COLORMASK_A;
    }
    else
    {
        PipelineStateGLES2Pool::Free(handle);
        handle = InvalidHandle;
    }

    return handle;
}

//------------------------------------------------------------------------------

static Handle gles2_PipelineState_CreateVertexConstBuffer(Handle ps, uint32 bufIndex)
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get(ps);

    return ps2->prog.vprog->InstanceConstBuffer(bufIndex);
}

//------------------------------------------------------------------------------

static Handle gles2_PipelineState_CreateFragmentConstBuffer(Handle ps, uint32 bufIndex)
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get(ps);

    return ps2->prog.fprog->InstanceConstBuffer(bufIndex);
}

namespace PipelineStateGLES2
{
void Init(uint32 maxCount)
{
    PipelineStateGLES2Pool::Reserve(maxCount);
}

void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PipelineState_Create = &gles2_PipelineState_Create;
    dispatch->impl_PipelineState_Delete = &gles2_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer = &gles2_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer = &gles2_PipelineState_CreateFragmentConstBuffer;
}

void SetToRHI(Handle ps)
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get(ps);

    DVASSERT(ps2);

    glState.UseProgram(ps2->prog.glProg);
    VertexDeclGLES2::InvalidateVAttrCache();

    if (ps2->blendEnabled)
    {
        glState.SetEnabled(OpenGLState::StateBlend, OpenGLState::Enabled);

        if (ps2->blendSrc != cachedBlendSrc || ps2->blendDst != cachedBlendDst)
        {
            GL_CALL(glBlendFunc(ps2->blendSrc, ps2->blendDst));
            cachedBlendSrc = ps2->blendSrc;
            cachedBlendDst = ps2->blendDst;
        }
    }
    else
    {
        glState.SetEnabled(OpenGLState::StateBlend, OpenGLState::Disabled);
    }

    glState.SetColorMask(ps2->maskR, ps2->maskG, ps2->maskB, ps2->maskA);
}

void SetVertexDeclToRHI(Handle ps, uint32 layoutUID, uint32 firstVertex, uint32 vertexStreamCount, const Handle* vb)
{
    //Trace("SetVertexDeclToRHI  layoutUID= %u\n",layoutUID);
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get(ps);

    if (layoutUID != VertexLayout::InvalidUID)
    {
        bool do_add = true;

        for (std::vector<PipelineStateGLES2_t::VertexProgGLES2::vdecl_t>::iterator v = ps2->prog.vprog->altVdecl.begin(), v_end = ps2->prog.vprog->altVdecl.end(); v != v_end; ++v)
        {
            if (v->layoutUID == layoutUID)
            {
                do_add = false;
                break;
            }
        }

        if (do_add)
        {
            const VertexLayout* layout = VertexLayout::Get(layoutUID);
            PipelineStateGLES2_t::VertexProgGLES2::vdecl_t vdecl;

            vdecl.layoutUID = layoutUID;
            vdecl.vdecl.Construct(*layout);
            vdecl.vdecl.InitVattr(ps2->prog.glProg, true);
            ps2->prog.vprog->altVdecl.push_back(vdecl);
        }
    }

    ps2->prog.vprog->SetToRHI(layoutUID, firstVertex, vertexStreamCount, vb);
}

uint32 VertexSamplerCount(Handle ps)
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get(ps);
    return ps2->prog.vprog->SamplerCount();
}

uint32 ProgramUid(Handle ps)
{
    PipelineStateGLES2_t* ps2 = PipelineStateGLES2Pool::Get(ps);

    return ps2->prog.glProg;
}

void InvalidateCache()
{
    cachedBlendSrc = 0;
    cachedBlendDst = 0;
    VertexDeclGLES2::InvalidateVAttrCacheForTools();
}

void InvalidateVattrCache()
{
    VertexDeclGLES2::InvalidateVAttrCache();
}

} // namespace PipelineStateGLES2

} // namespace rhi
