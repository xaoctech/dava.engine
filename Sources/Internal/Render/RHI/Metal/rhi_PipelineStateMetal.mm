#include "../Common/rhi_Private.h"
#include "../Common/rhi_BackendImpl.h"
#include "../rhi_ShaderCache.h"
#include "../Common/rhi_Pool.h"
#include "rhi_RingBufferMetal.h"

#include "Logger/Logger.h"
using DAVA::Logger;

#include "FileSystem/File.h"

#include "_metal.h"

#define MTL_SHOW_SHADER_WARNINGS 0

#if !(TARGET_IPHONE_SIMULATOR == 1)

namespace rhi
{
static inline unsigned _Metal_VertexAttribIndex(VertexSemantics s, uint32 i)
{
    unsigned attr_i = DAVA::InvalidIndex;

    switch (s)
    {
    case VS_POSITION:
    {
        switch (i)
        {
        case 0:
            attr_i = VATTR_POSITION_0;
            break;
        case 1:
            attr_i = VATTR_POSITION_1;
            break;
        case 2:
            attr_i = VATTR_POSITION_2;
            break;
        case 3:
            attr_i = VATTR_POSITION_3;
            break;
        }
    }
    break;
    case VS_NORMAL:
    {
        switch (i)
        {
        case 0:
            attr_i = VATTR_NORMAL_0;
            break;
        case 1:
            attr_i = VATTR_NORMAL_1;
            break;
        case 2:
            attr_i = VATTR_NORMAL_2;
            break;
        case 3:
            attr_i = VATTR_NORMAL_3;
            break;
        }
    }
    break;
    case VS_TEXCOORD:
    {
        switch (i)
        {
        case 0:
            attr_i = VATTR_TEXCOORD_0;
            break;
        case 1:
            attr_i = VATTR_TEXCOORD_1;
            break;
        case 2:
            attr_i = VATTR_TEXCOORD_2;
            break;
        case 3:
            attr_i = VATTR_TEXCOORD_3;
            break;
        case 4:
            attr_i = VATTR_TEXCOORD_4;
            break;
        case 5:
            attr_i = VATTR_TEXCOORD_5;
            break;
        case 6:
            attr_i = VATTR_TEXCOORD_6;
            break;
        case 7:
            attr_i = VATTR_TEXCOORD_7;
            break;
        }
    }
    break;
    case VS_COLOR:
    {
        switch (i)
        {
        case 0:
            attr_i = VATTR_COLOR_0;
            break;
        case 1:
            attr_i = VATTR_COLOR_1;
            break;
        }
    }
    break;
    case VS_TANGENT:
        attr_i = VATTR_TANGENT;
        break;
    case VS_BINORMAL:
        attr_i = VATTR_BINORMAL;
        break;
    case VS_BLENDWEIGHT:
        attr_i = VATTR_BLENDWEIGHT;
        break;
    case VS_BLENDINDEX:
        attr_i = VATTR_BLENDINDEX;
        break;

    default:
        break;
    }

    DVASSERT(attr_i != DAVA::InvalidIndex);
    return attr_i;
}

//------------------------------------------------------------------------------

static void DumpShaderText(const char* code, unsigned code_sz)
{
    char src[64 * 1024];
    char* src_line[1024];
    unsigned line_cnt = 0;

    if (code_sz < sizeof(src))
    {
        memcpy(src, code, code_sz);
        src[code_sz] = '\0';
        memset(src_line, 0, sizeof(src_line));

        src_line[line_cnt++] = src;
        for (char* s = src; *s;)
        {
            if (*s == '\n')
            {
                *s = 0;
                ++s;

                while (*s && (/**s == '\n'  ||  */ *s == '\r'))
                {
                    *s = 0;
                    ++s;
                }

                if (!(*s))
                    break;

                src_line[line_cnt] = s;
                ++line_cnt;
            }
            else if (*s == '\r')
            {
                *s = ' ';
            }
            else
            {
                ++s;
            }
        }

        for (unsigned i = 0; i != line_cnt; ++i)
        {
            Logger::Info("%4u |  %s", 1 + i, src_line[i]);
        }
    }
    else
    {
        Logger::Info(code);
    }
}

//------------------------------------------------------------------------------

struct PipelineStateMetal_t
{
    struct ConstBuf
    {
        enum ProgType
        {
            PROG_VERTEX,
            PROG_FRAGMENT
        };

        ~ConstBuf()
        {
            Destroy();
        }

        bool Construct(ProgType type, unsigned index, unsigned count);
        void Destroy();

        unsigned ConstCount() const;
        bool SetConst(unsigned const_i, unsigned count, const float* cdata);
        bool SetConst(unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count);

        unsigned Instance();
        void SetToRHI(unsigned bufIndex, id<MTLRenderCommandEncoder> ce);
        void SetToRHI(unsigned bufIndex, unsigned instOffset, id<MTLRenderCommandEncoder> ce);
        void InvalidateInst();

    private:
        ProgType type = ProgType::PROG_VERTEX;
        unsigned index = DAVA::InvalidIndex;
        unsigned count = 0;
        float* data = nullptr;
        float* inst = nullptr;
        unsigned inst_offset = 0;
    };

    struct VertexProg
    {
        void GetBufferInfo(MTLRenderPipelineReflection* info);
        Handle InstanceConstBuffer(unsigned buf_i);
        struct BufInfo
        {
            unsigned index;
            unsigned count;
            unsigned used;
        };
        BufInfo cbuf[MAX_CONST_BUFFER_COUNT];
    };

    struct FragmentProg
    {
        void GetBufferInfo(MTLRenderPipelineReflection* info);
        Handle InstanceConstBuffer(unsigned buf_i);
        struct BufInfo
        {
            unsigned index;
            unsigned count;
            unsigned used;
        };
        BufInfo cbuf[MAX_CONST_BUFFER_COUNT];
    };

    MTLRenderPipelineDescriptor* baseDescriptor = nil;
    VertexProg vprog;
    FragmentProg fprog;
    VertexLayout layout;

    struct state_t
    {
        uint32 layoutUID;
        MTLPixelFormat color_format[MAX_RENDER_TARGET_COUNT];
        uint32 color_count;
        id<MTLRenderPipelineState> state;
        uint32 stride;
        uint32 sampleCount;
        uint32 depth_used : 1;
        uint32 stencil_used : 1;
    };
    std::vector<state_t> altState;
};

typedef ResourcePool<PipelineStateMetal_t, RESOURCE_PIPELINE_STATE, PipelineState::Descriptor, false> PipelineStateMetalPool;
typedef ResourcePool<PipelineStateMetal_t::ConstBuf, RESOURCE_CONST_BUFFER, ConstBuffer::Descriptor, false> ConstBufMetalPool;

RHI_IMPL_POOL(PipelineStateMetal_t, RESOURCE_PIPELINE_STATE, PipelineState::Descriptor, false);
RHI_IMPL_POOL_SIZE(PipelineStateMetal_t::ConstBuf, RESOURCE_CONST_BUFFER, ConstBuffer::Descriptor, false, 12 * 1024);

static RingBufferMetal DefaultConstRingBuffer;
static RingBufferMetal VertexConstRingBuffer;
static RingBufferMetal FragmentConstRingBuffer;

//------------------------------------------------------------------------------

void PipelineStateMetal_t::FragmentProg::GetBufferInfo(MTLRenderPipelineReflection* info)
{
    for (unsigned i = 0; i != countof(cbuf); ++i)
    {
        cbuf[i].index = i;
        cbuf[i].count = 0;
        cbuf[i].used = false;

        for (MTLArgument* arg in info.fragmentArguments)
        {
            if (arg.active && arg.type == MTLArgumentTypeBuffer && arg.index == i)
            {
                MTLStructType* str = arg.bufferStructType;

                for (MTLStructMember* member in str.members)
                {
                    if (member.dataType == MTLDataTypeArray)
                    {
                        MTLArrayType* arr = member.arrayType;

                        cbuf[i].index = i;
                        cbuf[i].count = arr.arrayLength;
                        cbuf[i].used = true;
                        break;
                    }
                    else if (member.dataType == MTLDataTypeFloat4)
                    {
                        cbuf[i].index = i;
                        cbuf[i].count = 1;
                        cbuf[i].used = true;
                        break;
                    }
                }

                break;
            }
        }
    }
}

//------------------------------------------------------------------------------

Handle
PipelineStateMetal_t::FragmentProg::InstanceConstBuffer(unsigned bufIndex)
{
    Handle handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
    //    DVASSERT(cbuf[bufIndex].location != DAVA::InvalidIndex);

    if (bufIndex < countof(cbuf) && cbuf[bufIndex].index != DAVA::InvalidIndex)
    {
        handle = ConstBufMetalPool::Alloc();

        ConstBuf* cb = ConstBufMetalPool::Get(handle);

        if (!cb->Construct(ConstBuf::PROG_FRAGMENT, cbuf[bufIndex].index, cbuf[bufIndex].count))
        {
            ConstBufMetalPool::Free(handle);
            handle = InvalidHandle;
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

void PipelineStateMetal_t::VertexProg::GetBufferInfo(MTLRenderPipelineReflection* info)
{
    for (unsigned i = 0; i != countof(cbuf); ++i)
    {
        cbuf[i].index = i;
        cbuf[i].count = 0;
        cbuf[i].used = false;

        for (MTLArgument* arg in info.vertexArguments)
        {
            //            const char* name = arg.name.UTF8String;
            if (arg.active && arg.type == MTLArgumentTypeBuffer && arg.index == MAX_VERTEX_STREAM_COUNT + i // vprog-buf[0..MAX_VERTEX_STREAM_COUNT] assumed to be vdata
                )
            {
                MTLStructType* str = arg.bufferStructType;

                for (MTLStructMember* member in str.members)
                {
                    if (member.dataType == MTLDataTypeArray)
                    {
                        MTLArrayType* arr = member.arrayType;

                        cbuf[i].index = i;
                        cbuf[i].count = arr.arrayLength;
                        cbuf[i].used = true;
                        break;
                    }
                    else if (member.dataType == MTLDataTypeFloat4)
                    {
                        cbuf[i].index = i;
                        cbuf[i].count = 1;
                        cbuf[i].used = true;
                        break;
                    }
                }

                break;
            }
        }
    }
}

//------------------------------------------------------------------------------

Handle
PipelineStateMetal_t::VertexProg::InstanceConstBuffer(unsigned bufIndex)
{
    Handle handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
    //    DVASSERT(cbuf[bufIndex].location != DAVA::InvalidIndex);

    if (bufIndex < countof(cbuf) && cbuf[bufIndex].index != DAVA::InvalidIndex)
    {
        handle = ConstBufMetalPool::Alloc();

        ConstBuf* cb = ConstBufMetalPool::Get(handle);

        if (!cb->Construct(ConstBuf::PROG_VERTEX, cbuf[bufIndex].index, cbuf[bufIndex].count))
        {
            ConstBufMetalPool::Free(handle);
            handle = InvalidHandle;
        }
    }

    return handle;
}

//------------------------------------------------------------------------------

bool PipelineStateMetal_t::ConstBuf::Construct(PipelineStateMetal_t::ConstBuf::ProgType ptype, unsigned buf_i, unsigned cnt)
{
    Destroy();

    type = ptype;
    index = buf_i;
    count = cnt;
    data = reinterpret_cast<float*>(malloc(cnt * 4 * sizeof(float)));
    inst = nullptr;
    inst_offset = 0;

    return true;
}

//------------------------------------------------------------------------------

void PipelineStateMetal_t::ConstBuf::Destroy()
{
    if (data)
    {
        ::free(data);
        data = nullptr;
    }

    index = DAVA::InvalidIndex;
    count = 0;
    inst = nullptr;
    inst_offset = 0;
}

//------------------------------------------------------------------------------

unsigned
PipelineStateMetal_t::ConstBuf::ConstCount() const
{
    return count;
}

//------------------------------------------------------------------------------

bool PipelineStateMetal_t::ConstBuf::SetConst(unsigned const_i, unsigned const_count, const float* cdata)
{
    bool success = false;

    if (const_i + const_count <= count)
    {
        memcpy(data + const_i * 4, cdata, const_count * 4 * sizeof(float));
        inst = nullptr;
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

bool PipelineStateMetal_t::ConstBuf::SetConst(unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count)
{
    bool success = false;

    if (const_i <= count && const_sub_i < 4)
    {
        memcpy(data + const_i * 4 + const_sub_i, cdata, data_count * sizeof(float));
        inst = nullptr;
        success = true;
    }

    return success;
}

//------------------------------------------------------------------------------

void PipelineStateMetal_t::ConstBuf::SetToRHI(unsigned bufIndex, id<MTLRenderCommandEncoder> ce)
{
    id<MTLBuffer> buf = DefaultConstRingBuffer.BufferUID();

    if (!inst)
    {
        inst = DefaultConstRingBuffer.Alloc(count * 4 * sizeof(float), &inst_offset);
        //        inst = (type == PROG_VERTEX)
        //               ? VertexConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset )
        //               : FragmentConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset );

        memcpy(inst, data, count * 4 * sizeof(float));
    }

    if (type == PROG_VERTEX)
        [ce setVertexBuffer:buf offset:inst_offset atIndex:MAX_VERTEX_STREAM_COUNT + bufIndex]; // vprog-buf[0..MAX_VERTEX_STREAM_COUNT] assumed to be vdata
    else
        [ce setFragmentBuffer:buf offset:inst_offset atIndex:bufIndex];
}

//------------------------------------------------------------------------------

void PipelineStateMetal_t::ConstBuf::SetToRHI(unsigned bufIndex, unsigned instOffset, id<MTLRenderCommandEncoder> ce)
{
    id<MTLBuffer> buf = DefaultConstRingBuffer.BufferUID();

    if (type == PROG_VERTEX)
        [ce setVertexBuffer:buf offset:instOffset atIndex:MAX_VERTEX_STREAM_COUNT + bufIndex]; // vprog-buf[0..MAX_VERTEX_STREAM_COUNT] assumed to be vdata
    else
        [ce setFragmentBuffer:buf offset:instOffset atIndex:bufIndex];
}

//------------------------------------------------------------------------------

unsigned
PipelineStateMetal_t::ConstBuf::Instance()
{
    if (!inst)
    {
        inst = DefaultConstRingBuffer.Alloc(count * 4 * sizeof(float), &inst_offset);
        //        inst = (type == PROG_VERTEX)
        //               ? VertexConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset )
        //               : FragmentConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset );

        memcpy(inst, data, count * 4 * sizeof(float));
    }

    return inst_offset;
}

//------------------------------------------------------------------------------

void PipelineStateMetal_t::ConstBuf::InvalidateInst()
{
    inst = nullptr;
}

//==============================================================================

static Handle metal_PipelineState_Create(const PipelineState::Descriptor& desc)
{
    Handle handle = PipelineStateMetalPool::Alloc();
    PipelineStateMetal_t* ps = PipelineStateMetalPool::Get(handle);
    const std::vector<uint8>& vprog_bin = rhi::ShaderCache::GetProg(desc.vprogUid);
    const std::vector<uint8>& fprog_bin = rhi::ShaderCache::GetProg(desc.fprogUid);

    NSString* vp_src = [NSString stringWithUTF8String:reinterpret_cast<const char*>(vprog_bin.data())];
    MTLCompileOptions* vp_opt = [[MTLCompileOptions alloc] init];
    NSError* vp_err = nil;
    id<MTLLibrary> vp_lib = [_Metal_Device newLibraryWithSource:vp_src options:vp_opt error:&vp_err];
    id<MTLFunction> vp_func = nil;

    if (vp_lib != nil)
    {
        vp_func = [vp_lib newFunctionWithName:@"vp_main"];

        if (vp_func == nil)
        {
            Logger::Error("FAILED to get vprog \"%s\" function", desc.vprogUid.c_str());
        }
    }
    else
    {
        DumpShaderText(reinterpret_cast<const char*>(vprog_bin.data()), vprog_bin.size());
        Logger::Error("FAILED to compile vprog \"%s\" :", desc.vprogUid.c_str());
        Logger::Error("  %s", (vp_err != nil) ? vp_err.localizedDescription.UTF8String : "<unknown error>");
    }

    #if MTL_SHOW_SHADER_WARNINGS
    if (vp_err != nil)
        Logger::Warning("vprog warnings:\n %s ", vp_err.localizedDescription.UTF8String);
    #endif
    [vp_opt release];
    [vp_lib release];

    // compile fprog

    NSString* fp_src = [NSString stringWithUTF8String:reinterpret_cast<const char*>(fprog_bin.data())];
    MTLCompileOptions* fp_opt = [[MTLCompileOptions alloc] init];
    NSError* fp_err = nil;
    id<MTLLibrary> fp_lib = [_Metal_Device newLibraryWithSource:fp_src options:fp_opt error:&fp_err];
    id<MTLFunction> fp_func = nil;

    if (fp_lib != nil)
    {
        fp_func = [fp_lib newFunctionWithName:@"fp_main"];

        if (fp_func == nil)
        {
            Logger::Error("FAILED to get fprog \"%s\" function", desc.fprogUid.c_str());
        }
    }
    else
    {
        DumpShaderText(reinterpret_cast<const char*>(fprog_bin.data()), fprog_bin.size());
        Logger::Error("FAILED to compile fprog \"%s\" :", desc.fprogUid.c_str());
        Logger::Error("  %s", (fp_err != nil) ? fp_err.localizedDescription.UTF8String : "<unknown error>");
    }
    
    #if MTL_SHOW_SHADER_WARNINGS
    if (fp_err != nil)
        Logger::Warning("fprog warnings:\n %s ", fp_err.localizedDescription.UTF8String);
    #endif
    [fp_opt release];
    [fp_lib release];

    if (vp_func != nil && fp_func != nil)
    {
        ps->baseDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        MTLRenderPipelineReflection* ps_info = nil;
        NSError* rs_err = nil;

        ps->baseDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        ps->baseDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatStencil8;
        ps->baseDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        ps->baseDescriptor.colorAttachments[0].blendingEnabled = desc.blending.rtBlend[0].blendEnabled;
        ps->baseDescriptor.vertexFunction = vp_func;
        ps->baseDescriptor.fragmentFunction = fp_func;
        ps->baseDescriptor.sampleCount = 1;

        if (desc.blending.rtBlend[0].blendEnabled)
        {
            switch (desc.blending.rtBlend[0].colorSrc)
            {
            case BLENDOP_ZERO:
                ps->baseDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorZero;
                ps->baseDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
                break;

            case BLENDOP_ONE:
                ps->baseDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
                ps->baseDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
                break;

            case BLENDOP_SRC_ALPHA:
                ps->baseDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
                ps->baseDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                break;

            case BLENDOP_INV_SRC_ALPHA:
                ps->baseDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                ps->baseDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                break;

            case BLENDOP_SRC_COLOR:
                ps->baseDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceColor;
                ps->baseDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceColor;
                break;

            case BLENDOP_DST_COLOR:
                ps->baseDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
                ps->baseDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorDestinationColor;
                break;
            }

            switch (desc.blending.rtBlend[0].colorDst)
            {
            case BLENDOP_ZERO:
                ps->baseDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorZero;
                ps->baseDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;
                break;

            case BLENDOP_ONE:
                ps->baseDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
                ps->baseDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
                break;

            case BLENDOP_SRC_ALPHA:
                ps->baseDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorSourceAlpha;
                ps->baseDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                break;

            case BLENDOP_INV_SRC_ALPHA:
                ps->baseDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                ps->baseDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                break;

            case BLENDOP_SRC_COLOR:
                ps->baseDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorSourceColor;
                ps->baseDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorSourceColor;
                break;

            case BLENDOP_DST_COLOR:
                ps->baseDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorDestinationColor;
                ps->baseDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorDestinationColor;
                break;
            }
        }

        ps->baseDescriptor.colorAttachments[0].writeMask = MTLColorWriteMaskNone;

        if (desc.blending.rtBlend[0].writeMask & COLORMASK_R)
            ps->baseDescriptor.colorAttachments[0].writeMask |= MTLColorWriteMaskRed;
        if (desc.blending.rtBlend[0].writeMask & COLORMASK_G)
            ps->baseDescriptor.colorAttachments[0].writeMask |= MTLColorWriteMaskGreen;
        if (desc.blending.rtBlend[0].writeMask & COLORMASK_B)
            ps->baseDescriptor.colorAttachments[0].writeMask |= MTLColorWriteMaskBlue;
        if (desc.blending.rtBlend[0].writeMask & COLORMASK_A)
            ps->baseDescriptor.colorAttachments[0].writeMask |= MTLColorWriteMaskAlpha;

        for (unsigned i = 0; i != desc.vertexLayout.ElementCount(); ++i)
        {
            unsigned attr_i = _Metal_VertexAttribIndex(desc.vertexLayout.ElementSemantics(i), desc.vertexLayout.ElementSemanticsIndex(i));
            MTLVertexFormat fmt = MTLVertexFormatInvalid;

            switch (desc.vertexLayout.ElementDataType(i))
            {
            case VDT_FLOAT:
            {
                switch (desc.vertexLayout.ElementDataCount(i))
                {
                case 1:
                    fmt = MTLVertexFormatFloat;
                    break;
                case 2:
                    fmt = MTLVertexFormatFloat2;
                    break;
                case 3:
                    fmt = MTLVertexFormatFloat3;
                    break;
                case 4:
                    fmt = MTLVertexFormatFloat4;
                    break;
                }
            }
            break;

            case VDT_UINT8:
            case VDT_UINT8N:
            {
                switch (desc.vertexLayout.ElementDataCount(i))
                {
                //                        case 1 : fmt = MTLVertexFormatUCharNormalized; break;
                case 2:
                    fmt = MTLVertexFormatUChar2Normalized;
                    break;
                case 3:
                    fmt = MTLVertexFormatUChar3Normalized;
                    break;
                case 4:
                    fmt = MTLVertexFormatUChar4Normalized;
                    break;
                }
            }
            break;

            default:
                break;
            }
            DVASSERT(fmt != MTLVertexFormatInvalid);

            ps->baseDescriptor.vertexDescriptor.attributes[attr_i].bufferIndex = desc.vertexLayout.ElementStreamIndex(i);
            ps->baseDescriptor.vertexDescriptor.attributes[attr_i].offset = desc.vertexLayout.ElementOffset(i);
            ps->baseDescriptor.vertexDescriptor.attributes[attr_i].format = fmt;
        }

        for (unsigned s = 0; s != desc.vertexLayout.StreamCount(); ++s)
        {
            ps->baseDescriptor.vertexDescriptor.layouts[s].stepFunction = (desc.vertexLayout.StreamFrequency(s) == VDF_PER_VERTEX) ? MTLVertexStepFunctionPerVertex : MTLVertexStepFunctionPerInstance;
            ps->baseDescriptor.vertexDescriptor.layouts[s].stepRate = 1;
            ps->baseDescriptor.vertexDescriptor.layouts[s].stride = desc.vertexLayout.Stride(s);
        }

        for (uint32 i = 0; i < MAX_RENDER_TARGET_COUNT; i++)
        {
            ps->baseDescriptor.colorAttachments[i].pixelFormat = MTLPixelFormatRGBA8Unorm;
        }

        id<MTLRenderPipelineState> baseState = [_Metal_Device newRenderPipelineStateWithDescriptor:ps->baseDescriptor options:MTLPipelineOptionBufferTypeInfo reflection:&ps_info error:&rs_err];
        if (baseState != nil)
        {
            ps->vprog.GetBufferInfo(ps_info);
            ps->fprog.GetBufferInfo(ps_info);
            ps->layout = desc.vertexLayout;
        }
        else
        {
            Logger::Error("FAILED create pipeline-state :\n%s", (rs_err != nil) ? rs_err.localizedDescription.UTF8String : "<unspecified error>");
            Logger::Info("  vprog-uid = %s", desc.vprogUid.c_str());
            Logger::Info("  fprog-uid = %s", desc.fprogUid.c_str());
            DVASSERT(false);
        }
        [baseState release];
    }
    else
    {
        handle = InvalidHandle;
    }

    return handle;
}

//------------------------------------------------------------------------------

static void
metal_PipelineState_Delete(Handle ps)
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get(ps);

    if (psm)
    {
        PipelineStateMetalPool::Free(ps);
    }
}

//------------------------------------------------------------------------------

static Handle
metal_PipelineState_CreateVertexConstBuffer(Handle ps, unsigned bufIndex)
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get(ps);

    return psm->vprog.InstanceConstBuffer(bufIndex);
}

//------------------------------------------------------------------------------

static Handle
metal_PipelineState_CreateFragmentConstBuffer(Handle ps, unsigned bufIndex)
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get(ps);

    return psm->fprog.InstanceConstBuffer(bufIndex);
}

static bool
metal_ConstBuffer_SetConst(Handle cb, uint32 constIndex, uint32 constCount, const float* data)
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get(cb);

    return buf->SetConst(constIndex, constCount, data);
}

static bool
metal_ConstBuffer_SetConst1fv(Handle cb, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount)
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get(cb);

    return buf->SetConst(constIndex, constSubIndex, data, dataCount);
}

static void
metal_ConstBuffer_Delete(Handle cb)
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get(cb);

    buf->Destroy();
    ConstBufMetalPool::Free(cb);
}

namespace ConstBufferMetal
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_ConstBuffer_SetConst = &metal_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv = &metal_ConstBuffer_SetConst1fv;
    dispatch->impl_ConstBuffer_Delete = &metal_ConstBuffer_Delete;
}

void
ResetRingBuffer()
{
    DefaultConstRingBuffer.Reset();
}
}

namespace PipelineStateMetal
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_PipelineState_Create = &metal_PipelineState_Create;
    dispatch->impl_PipelineState_Delete = &metal_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer = &metal_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer = &metal_PipelineState_CreateFragmentConstBuffer;
}

uint32
VertexStreamCount(Handle ps)
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get(ps);

    return psm->layout.StreamCount();
}

uint32 SetToRHI(Handle ps, uint32 layoutUID, const MTLPixelFormat* color_fmt, unsigned color_count, bool depth_used, bool stencil_used,
                id<MTLRenderCommandEncoder> ce, uint32 sampleCount)
{
    uint32 stride = 0;
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get(ps);

    DVASSERT(psm);

    bool do_add = true;
    unsigned si = DAVA::InvalidIndex;

    for (unsigned i = 0; i != psm->altState.size(); ++i)
    {
        bool color_fmt_match = false;

        if (psm->altState[i].color_count == color_count)
        {
            color_fmt_match = true;
            for (unsigned t = 0; t != color_count; ++t)
            {
                if (psm->altState[i].color_format[t] != color_fmt[t])
                {
                    color_fmt_match = false;
                    break;
                }
            }
        }

        if ((psm->altState[i].layoutUID == layoutUID) &&
            color_fmt_match &&
            (psm->altState[i].depth_used == depth_used) &&
            (psm->altState[i].stencil_used == stencil_used) &&
            (psm->altState[i].sampleCount == sampleCount))
        {
            si = i;
            do_add = false;
            break;
        }
    }

    if (do_add)
    {
        const VertexLayout* layout = (layoutUID != VertexLayout::InvalidUID) ? VertexLayout::Get(layoutUID) : &psm->layout;
        MTLRenderPipelineDescriptor* rp_desc = [MTLRenderPipelineDescriptor new];
        MTLRenderPipelineReflection* ps_info = nil;
        NSError* rs_err = nil;

        rp_desc.depthAttachmentPixelFormat = depth_used ? MTLPixelFormatDepth32Float : MTLPixelFormatInvalid;
        rp_desc.stencilAttachmentPixelFormat = stencil_used ? MTLPixelFormatStencil8 : MTLPixelFormatInvalid;

        for (unsigned t = 0; t != color_count; ++t)
        {
            rp_desc.colorAttachments[t] = psm->baseDescriptor.colorAttachments[t];
            rp_desc.colorAttachments[t].pixelFormat = color_fmt[t];
        }

        rp_desc.sampleCount = sampleCount;
        rp_desc.vertexFunction = psm->baseDescriptor.vertexFunction;
        rp_desc.fragmentFunction = psm->baseDescriptor.fragmentFunction;

        for (unsigned i = 0; i != psm->layout.ElementCount(); ++i)
        {
            unsigned attr_i = _Metal_VertexAttribIndex(psm->layout.ElementSemantics(i), psm->layout.ElementSemanticsIndex(i));
            bool attr_set = false;
            unsigned stream_i = layout->ElementStreamIndex(i);

            for (unsigned j = 0; j != layout->ElementCount(); ++j)
            {
                if (layout->ElementSemantics(j) == psm->layout.ElementSemantics(i) && layout->ElementSemanticsIndex(j) == psm->layout.ElementSemanticsIndex(i))
                {
                    MTLVertexFormat fmt = MTLVertexFormatInvalid;

                    switch (psm->layout.ElementDataType(i))
                    {
                    case VDT_FLOAT:
                    {
                        switch (psm->layout.ElementDataCount(i))
                        {
                        case 1:
                            fmt = MTLVertexFormatFloat;
                            break;
                        case 2:
                            fmt = MTLVertexFormatFloat2;
                            break;
                        case 3:
                            fmt = MTLVertexFormatFloat3;
                            break;
                        case 4:
                            fmt = MTLVertexFormatFloat4;
                            break;
                        }
                    }
                    break;

                    case VDT_HALF:
                    {
                        switch (psm->layout.ElementDataCount(i))
                        {
                        case 1:
                            // fmt = MTLVertexFormatHalf;
                            DVASSERT(!"Unsupported format");
                            break;
                        case 2:
                            fmt = MTLVertexFormatHalf2;
                            break;
                        case 3:
                            fmt = MTLVertexFormatHalf3;
                            break;
                        case 4:
                            fmt = MTLVertexFormatHalf4;
                            break;
                        }
                    }
                    break;

                    case VDT_UINT8:
                    case VDT_UINT8N:
                    {
                        switch (psm->layout.ElementDataCount(i))
                        {
                        //                                    case 1 : fmt = MTLVertexFormatUCharNormalized; break;
                        case 2:
                            fmt = MTLVertexFormatUChar2Normalized;
                            break;
                        case 3:
                            fmt = MTLVertexFormatUChar3Normalized;
                            break;
                        case 4:
                            fmt = MTLVertexFormatUChar4Normalized;
                            break;
                        }
                    }
                    break;

                    default:
                        break;
                    }

                    rp_desc.vertexDescriptor.attributes[attr_i].bufferIndex = stream_i;
                    rp_desc.vertexDescriptor.attributes[attr_i].offset = layout->ElementOffset(j);
                    rp_desc.vertexDescriptor.attributes[attr_i].format = fmt;

                    attr_set = true;
                    break;
                }
            }

            if (!attr_set)
            {
                Logger::Error("vertex-layout mismatch");
                Logger::Info("pipeline-state layout:");
                psm->layout.Dump();
                Logger::Info("packet layout:");
                layout->Dump();
                DVASSERT(!"kaboom!");
            }
        }

        for (unsigned s = 0; s != layout->StreamCount(); ++s)
        {
            rp_desc.vertexDescriptor.layouts[s].stepFunction = (layout->StreamFrequency(s) == VDF_PER_VERTEX) ? MTLVertexStepFunctionPerVertex : MTLVertexStepFunctionPerInstance;
            rp_desc.vertexDescriptor.layouts[s].stepRate = 1;
            rp_desc.vertexDescriptor.layouts[s].stride = layout->Stride(s);
        }

        PipelineStateMetal_t::state_t state;
        state.layoutUID = layoutUID;
        state.state = [_Metal_Device newRenderPipelineStateWithDescriptor:rp_desc options:MTLPipelineOptionNone reflection:&ps_info error:&rs_err];
        state.color_count = color_count;
        for (unsigned t = 0; t != color_count; ++t)
            state.color_format[t] = color_fmt[t];
        state.depth_used = depth_used;
        state.stencil_used = stencil_used;
        state.stride = layout->Stride();
        state.sampleCount = sampleCount;

        if (state.state != nil)
        {
            si = psm->altState.size();
            psm->altState.push_back(state);
        }
        else
        {
            if (rs_err != nil)
                Logger::Error("failed to create alt-ps : %s", rs_err.localizedDescription.UTF8String);
        }

        [rp_desc release];
    }

    DVASSERT(si != DAVA::InvalidIndex);

    PipelineStateMetal_t::state_t& stateObject = psm->altState[si];

    [ce setRenderPipelineState:stateObject.state];
    stride = stateObject.stride;

    return stride;
}

} // namespace PipelineStateMetal

namespace ConstBufferMetal
{
void Init(uint32 maxCount)
{
    ConstBufMetalPool::Reserve(maxCount);
}

void InitializeRingBuffer(uint32 size)
{
    DefaultConstRingBuffer.Initialize(size);
    //    VertexConstRingBuffer.Initialize( size );
    //    FragmentConstRingBuffer.Initialize( size );
}

void SetToRHI(Handle buf, unsigned bufIndex, id<MTLRenderCommandEncoder> ce)
{
    PipelineStateMetal_t::ConstBuf* cbuf = ConstBufMetalPool::Get(buf);

    cbuf->SetToRHI(bufIndex, ce);
}

void SetToRHI(Handle buf, unsigned bufIndex, unsigned instOffset, id<MTLRenderCommandEncoder> ce)
{
    PipelineStateMetal_t::ConstBuf* cbuf = ConstBufMetalPool::Get(buf);

    cbuf->SetToRHI(bufIndex, instOffset, ce);
}

unsigned
Instance(Handle buf)
{
    PipelineStateMetal_t::ConstBuf* cbuf = ConstBufMetalPool::Get(buf);

    return cbuf->Instance();
}

void InvalidateAllInstances()
{
    for (ConstBufMetalPool::Iterator b = ConstBufMetalPool::Begin(), b_end = ConstBufMetalPool::End(); b != b_end; ++b)
    {
        b->InvalidateInst();
    }
}
}

} // namespace rhi

#endif //#if !(TARGET_IPHONE_SIMULATOR==1)
