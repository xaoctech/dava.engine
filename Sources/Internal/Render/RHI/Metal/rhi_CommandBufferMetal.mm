#include "../Common/rhi_Private.h"
#include "../Common/rhi_Pool.h"
#include "rhi_Metal.h"

#include "../rhi_Type.h"
#include "../rhi_Public.h"
#include "../Common/dbg_StatSet.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
using DAVA::Logger;
#include "Debug/ProfilerCPU.h"
#include "../Common/rhi_CommonImpl.h"
#include "../Common/SoftwareCommandBuffer.h"

#include "_metal.h"

#define USE_SHARED_COMMAND_BUFFER 1
#define DISABLE_LOAD_STORE 0

#if !(TARGET_IPHONE_SIMULATOR == 1)
namespace rhi
{
struct RenderPassMetal_t
{
    RenderPassConfig cfg;
    MTLRenderPassDescriptor* desc;
    id<MTLCommandBuffer> commandBuffer;
    id<MTLParallelRenderCommandEncoder> encoder;
    std::vector<Handle> cmdBuf;
    int priority;    

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    bool Initialize();
#endif
};

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
class CommandBufferMetal_t
#else
class CommandBufferMetal_t : public SoftwareCommandBuffer
#endif
{
public:
    id<MTLRenderCommandEncoder> encoder;
    id<MTLCommandBuffer> buf;
    id<MTLTexture> rt;

    MTLPixelFormat color_fmt[MAX_RENDER_TARGET_COUNT];
    uint32 color_count;
    Handle cur_ib;
    unsigned cur_vstream_count;
    Handle cur_vb[MAX_VERTEX_STREAM_COUNT];
    Handle cur_ss;
    uint32 cur_stride;
    uint32 sampleCount;
    uint32 targetLevel;
    bool depth_used;
    bool stencil_used;

    void _ApplyVertexData(unsigned firstVertex = 0);
    
    #if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    void Execute();
    #endif
};

struct SyncObjectMetal_t
{
    uint32 is_signaled : 1;
};

typedef ResourcePool<CommandBufferMetal_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false> CommandBufferPoolMetal;
typedef ResourcePool<RenderPassMetal_t, RESOURCE_RENDER_PASS, RenderPassConfig, false> RenderPassPoolMetal;
typedef ResourcePool<SyncObjectMetal_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false> SyncObjectPoolMetal;

RHI_IMPL_POOL(CommandBufferMetal_t, RESOURCE_COMMAND_BUFFER, CommandBuffer::Descriptor, false);
RHI_IMPL_POOL(RenderPassMetal_t, RESOURCE_RENDER_PASS, RenderPassConfig, false);
RHI_IMPL_POOL(SyncObjectMetal_t, RESOURCE_SYNC_OBJECT, SyncObject::Descriptor, false);

static DAVA::Mutex _Metal_SyncObjectsSync;

static ResetParam _Metal_ResetParam;
static DAVA::Mutex _Metal_ResetSync;
static bool _Metal_ResetPending = false;
static const char* lastDebugMarker = nullptr;

id<CAMetalDrawable> _Metal_currentDrawable = nil;

#if (USE_SHARED_COMMAND_BUFFER)
id<MTLCommandBuffer> _Metal_currentCommandBuffer = nil;
#endif

void CommandBufferMetal_t::_ApplyVertexData(unsigned firstVertex)
{
    for (unsigned s = 0; s != cur_vstream_count; ++s)
    {
        unsigned base = 0;
        id<MTLBuffer> vb = VertexBufferMetal::GetBuffer(cur_vb[s], &base);
        unsigned off = (s == 0) ? firstVertex * cur_stride : 0;

        [encoder setVertexBuffer:vb offset:base + off atIndex:s];
    }
}

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
void CommandBufferMetal_t::Execute()
{
    for (const uint8 *c = cmdData, *c_end = cmdData + curUsedSize; c != c_end;)
    {
        const SWCommand* cmd = reinterpret_cast<const SWCommand*>(c);

        switch (SoftwareCommandType(cmd->type))
        {
        case CMD_SET_MARKER:
        {
            lastDebugMarker = static_cast<const SWCommand_SetMarker*>(cmd)->text;
            break;
        }
        case CMD_BEGIN:
        {
            lastDebugMarker = "CMD_BEGIN";
            cur_vstream_count = 0;
            for (unsigned s = 0; s != countof(cur_vb); ++s)
                cur_vb[s] = InvalidHandle;

            [encoder setDepthStencilState:_Metal_DefDepthState];
        }
        break;

        case CMD_END:
        {
            lastDebugMarker = "CMD_END";
            [encoder endEncoding];

            Handle syncObject = static_cast<const SWCommand_End*>(cmd)->syncObject;

            if (syncObject != InvalidHandle)
            {
                [buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb) {
                  SyncObjectMetal_t* sync = SyncObjectPoolMetal::Get(syncObject);

                  sync->is_signaled = true;
                }];
            }
        }
        break;

        case CMD_SET_PIPELINE_STATE:
        {
            Handle ps = static_cast<const SWCommand_SetPipelineState*>(cmd)->ps;
            unsigned layoutUID = static_cast<const SWCommand_SetPipelineState*>(cmd)->vdecl;

            cur_stride = PipelineStateMetal::SetToRHI(ps, layoutUID, color_fmt, color_count, depth_used, stencil_used, encoder, sampleCount);
            cur_vstream_count = PipelineStateMetal::VertexStreamCount(ps);
            StatSet::IncStat(stat_SET_PS, 1);
        }
        break;

        case CMD_SET_CULL_MODE:
        {
            switch (CullMode(static_cast<const SWCommand_SetCullMode*>(cmd)->mode))
            {
            case CULL_NONE:
                [encoder setCullMode:MTLCullModeNone];
                break;

            case CULL_CCW:
                [encoder setFrontFacingWinding:MTLWindingClockwise];
                [encoder setCullMode:MTLCullModeBack];
                break;

            case CULL_CW:
                [encoder setFrontFacingWinding:MTLWindingClockwise];
                [encoder setCullMode:MTLCullModeFront];
                break;
            }
        }
        break;

        case CMD_SET_SCISSOR_RECT:
        {
            int x = static_cast<const SWCommand_SetScissorRect*>(cmd)->x;
            int y = static_cast<const SWCommand_SetScissorRect*>(cmd)->y;
            int w = static_cast<const SWCommand_SetScissorRect*>(cmd)->width;
            int h = static_cast<const SWCommand_SetScissorRect*>(cmd)->height;
            MTLScissorRect rc;

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                unsigned max_x = (rt) ? unsigned(rt.width >> targetLevel) : unsigned(_Metal_DefFrameBuf.width);
                unsigned max_y = (rt) ? unsigned(rt.height >> targetLevel) : unsigned(_Metal_DefFrameBuf.height);

                rc.x = x;
                rc.y = y;
                rc.width = (x + w > max_x) ? (max_x - rc.x) : w;
                rc.height = (y + h > max_y) ? (max_y - rc.y) : h;

                if (rc.width == 0)
                {
                    rc.width = 1;
                    if (rc.x > 0)
                        --rc.x;
                }

                if (rc.height == 0)
                {
                    rc.height = 1;
                    if (rc.y > 0)
                        --rc.y;
                }
            }
            else
            {
                rc.x = 0;
                rc.y = 0;
                if (rt)
                {
                    rc.width = rt.width >> targetLevel;
                    rc.height = rt.height >> targetLevel;
                }
                else
                {
                    rc.width = _Metal_DefFrameBuf.width;
                    rc.height = _Metal_DefFrameBuf.height;
                }
            }

            [encoder setScissorRect:rc];
        }
        break;

        case CMD_SET_VIEWPORT:
        {
            MTLViewport vp;
            int x = static_cast<const SWCommand_SetViewport*>(cmd)->x;
            int y = static_cast<const SWCommand_SetViewport*>(cmd)->y;
            int w = static_cast<const SWCommand_SetViewport*>(cmd)->width;
            int h = static_cast<const SWCommand_SetViewport*>(cmd)->height;

            if (!(x == 0 && y == 0 && w == 0 && h == 0))
            {
                vp.originX = x;
                vp.originY = y;
                vp.width = w;
                vp.height = h;
                vp.znear = 0.0;
                vp.zfar = 1.0;
            }
            else
            {
                vp.originX = 0;
                vp.originY = 0;
                vp.width = rt.width >> targetLevel;
                vp.height = rt.height >> targetLevel;
                vp.znear = 0.0;
                vp.zfar = 1.0;
            }

            [encoder setViewport:vp];
        }
        break;

        case CMD_SET_FILLMODE:
        {
            [encoder setTriangleFillMode:(FillMode(static_cast<const SWCommand_SetFillMode*>(cmd)->mode) == FILLMODE_WIREFRAME) ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];
        }
        break;

        case CMD_SET_VERTEX_DATA:
        {
            Handle vb = static_cast<const SWCommand_SetVertexData*>(cmd)->vb;
            unsigned streamIndex = static_cast<const SWCommand_SetVertexData*>(cmd)->streamIndex;

            cur_vb[streamIndex] = vb;
            StatSet::IncStat(stat_SET_VB, 1);
        }
        break;

        case CMD_SET_VERTEX_PROG_CONST_BUFFER:
        {
            Handle buffer = static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd)->buffer;
            unsigned index = static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd)->bufIndex;
            uintptr_t inst = reinterpret_cast<uintptr_t>(static_cast<const SWCommand_SetVertexProgConstBuffer*>(cmd)->inst);
            unsigned instOffset = static_cast<unsigned>(inst);

            ConstBufferMetal::SetToRHI(buffer, index, instOffset, encoder);
        }
        break;

        case CMD_SET_VERTEX_TEXTURE:
        {
            Handle tex = static_cast<const SWCommand_SetVertexTexture*>(cmd)->tex;
            unsigned unitIndex = static_cast<const SWCommand_SetVertexTexture*>(cmd)->unitIndex;

            const SamplerState::Descriptor::Sampler& sampler = SamplerStateMetal::GetVertexSampler(cur_ss, unitIndex);
            DVASSERT(DeviceCaps().textureFormat[TextureMetal::GetFormat(tex)].filterable ||
                     (sampler.minFilter != TEXFILTER_LINEAR && sampler.magFilter != TEXFILTER_LINEAR && sampler.minFilter != TEXMIPFILTER_LINEAR),
                     DAVA::Format("Texture format '%s' is non-filterable", TextureFormatToString(TextureMetal::GetFormat(tex))).c_str()
                     );
            
            TextureMetal::SetToRHIVertex(tex, unitIndex, encoder);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case CMD_SET_INDICES:
        {
            cur_ib = static_cast<const SWCommand_SetIndices*>(cmd)->ib;
            StatSet::IncStat(stat_SET_IB, 1);
        }
        break;

        case CMD_SET_QUERY_INDEX:
        {
            unsigned index = static_cast<const SWCommand_SetQueryIndex*>(cmd)->objectIndex;

            if (index != DAVA::InvalidIndex)
            {
                [encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:index * QueryBUfferElemeentAlign];
            }
            else
            {
                [encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
            }
        }
        break;

        case CMD_SET_QUERY_BUFFER:
            break; // do NOTHING

        case CMD_SET_FRAGMENT_PROG_CONST_BUFFER:
        {
            Handle buffer = static_cast<const SWCommand_SetFragmentProgConstBuffer*>(cmd)->buffer;
            unsigned index = static_cast<const SWCommand_SetFragmentProgConstBuffer*>(cmd)->bufIndex;
            uintptr_t inst = reinterpret_cast<uintptr_t>(static_cast<const SWCommand_SetFragmentProgConstBuffer*>(cmd)->inst);
            unsigned instOffset = static_cast<unsigned>(inst);
            ConstBufferMetal::SetToRHI(buffer, index, instOffset, encoder);
        }
        break;

        case CMD_SET_FRAGMENT_TEXTURE:
        {
            Handle tex = static_cast<const SWCommand_SetFragmentTexture*>(cmd)->tex;
            unsigned unitIndex = static_cast<const SWCommand_SetFragmentTexture*>(cmd)->unitIndex;

            const SamplerState::Descriptor::Sampler& sampler = SamplerStateMetal::GetFragmentSampler(cur_ss, unitIndex);
            TextureFormat texFormat = TextureMetal::GetFormat(tex);
            DVASSERT(DeviceCaps().textureFormat[texFormat].filterable ||
                     (sampler.minFilter != TEXFILTER_LINEAR && sampler.magFilter != TEXFILTER_LINEAR && sampler.minFilter != TEXMIPFILTER_LINEAR),
                     DAVA::Format("Texture format '%s' is non-filterable", TextureFormatToString(texFormat)).c_str()
                     );
            
            TextureMetal::SetToRHIFragment(tex, unitIndex, encoder);
            StatSet::IncStat(stat_SET_TEX, 1);
        }
        break;

        case CMD_SET_DEPTHSTENCIL_STATE:
        {
            DepthStencilStateMetal::SetToRHI(static_cast<const SWCommand_SetDepthStencilState*>(cmd)->depthStencilState, encoder);
        }
        break;

        case CMD_SET_SAMPLER_STATE:
        {
            cur_ss = static_cast<const SWCommand_SetSamplerState*>(cmd)->samplerState;
            SamplerStateMetal::SetToRHI(cur_ss, encoder);
            StatSet::IncStat(stat_SET_SS, 1);
        }
        break;

        case CMD_DRAW_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(static_cast<const SWCommand_DrawPrimitive*>(cmd)->mode);
            unsigned vertexCount = static_cast<const SWCommand_DrawPrimitive*>(cmd)->vertexCount;

            _ApplyVertexData();
            [encoder drawPrimitives:ptype vertexStart:0 vertexCount:vertexCount];
        }
        break;

        case CMD_DRAW_INDEXED_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->mode);
            unsigned indexCount = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->indexCount;
            unsigned firstVertex = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->firstVertex;
            unsigned startIndex = static_cast<const SWCommand_DrawIndexedPrimitive*>(cmd)->startIndex;

            unsigned ib_base = 0;
            id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cur_ib, &ib_base);
            MTLIndexType i_type = IndexBufferMetal::GetType(cur_ib);
            unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

            _ApplyVertexData(firstVertex);
            [encoder drawIndexedPrimitives:ptype indexCount:indexCount indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off];
        }
        break;

        case CMD_DRAW_INSTANCED_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->mode);
            unsigned vertexCount = static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->vertexCount;
            unsigned instCount = static_cast<const SWCommand_DrawInstancedPrimitive*>(cmd)->instanceCount;

            _ApplyVertexData();
            [encoder drawPrimitives:ptype vertexStart:0 vertexCount:vertexCount instanceCount:instCount];
        }
        break;

        case CMD_DRAW_INSTANCED_INDEXED_PRIMITIVE:
        {
            MTLPrimitiveType ptype = MTLPrimitiveType(static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->mode);
            unsigned firstVertex = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->firstVertex;
            unsigned indexCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->indexCount;
            unsigned startIndex = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->startIndex;
            unsigned instCount = static_cast<const SWCommand_DrawInstancedIndexedPrimitive*>(cmd)->instanceCount;

            unsigned ib_base = 0;
            id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cur_ib, &ib_base);
            MTLIndexType i_type = IndexBufferMetal::GetType(cur_ib);
            unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

            _ApplyVertexData(firstVertex);
            [encoder drawIndexedPrimitives:ptype indexCount:indexCount indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off instanceCount:instCount];
        }
        break;

        default:
            Logger::Error("unsupported command: %d", cmd->type);
            DVASSERT(false, "unsupported command");
        }

        if (cmd->type == CMD_END)
            break;
        c += cmd->size;
    }
}
#endif

//------------------------------------------------------------------------------

MTLLoadAction GetLoadActionValue(LoadAction la)
{
#if (DISABLE_LOAD_STORE)
    return MTLLoadActionDontCare;
#else
    switch (la)
    {
    case LOADACTION_CLEAR:
        return MTLLoadActionClear;
    case LOADACTION_LOAD:
        return MTLLoadActionLoad;
    default:
        return MTLLoadActionDontCare;
    }
#endif
}

MTLStoreAction GetStoreActionValue(StoreAction sa)
{
#if (DISABLE_LOAD_STORE)
    return MTLStoreActionDontCare;
#else
    switch (sa)
    {
    case STOREACTION_STORE:
        return MTLStoreActionStore;
    case STOREACTION_RESOLVE:
        return MTLStoreActionMultisampleResolve;
    default:
        return MTLStoreActionDontCare;
    }
#endif
}

void SetRenderPassAttachmentsWithDeduction(MTLRenderPassDescriptor* desc, const RenderPassConfig& cfg)
{
    for (size_t i = 0; i < countof(cfg.colorBuffer); ++i)
    {
        const RenderPassConfig::ColorBuffer& colorBuffer = cfg.colorBuffer[i];

        desc.colorAttachments[i].loadAction = GetLoadActionValue(colorBuffer.loadAction);
        desc.colorAttachments[i].storeAction = GetStoreActionValue(colorBuffer.storeAction);
        desc.colorAttachments[i].clearColor = MTLClearColorMake(colorBuffer.clearColor[0], colorBuffer.clearColor[1], colorBuffer.clearColor[2], colorBuffer.clearColor[3]);
        desc.colorAttachments[i].level = colorBuffer.textureLevel;

        int slice = 0;
        switch (colorBuffer.textureFace)
        {
        case TextureFace::TEXTURE_FACE_NONE:
        case TextureFace::TEXTURE_FACE_POSITIVE_X:
            slice = 0;
            break;
        case TextureFace::TEXTURE_FACE_NEGATIVE_X:
            slice = 1;
            break;
        case TextureFace::TEXTURE_FACE_POSITIVE_Y:
            slice = 2;
            break;
        case TextureFace::TEXTURE_FACE_NEGATIVE_Y:
            slice = 3;
            break;
        case TextureFace::TEXTURE_FACE_POSITIVE_Z:
            slice = 4;
            break;
        case TextureFace::TEXTURE_FACE_NEGATIVE_Z:
            slice = 5;
            break;
        default:
            DVASSERT(0, "Invalid texture face specified");
        }
        desc.colorAttachments[i].slice = slice;

        if (colorBuffer.texture != InvalidHandle)
        {
            if (cfg.UsingMSAA())
            {
                TextureMetal::SetAsRenderTarget(colorBuffer.multisampleTexture, desc);
                TextureMetal::SetAsResolveRenderTarget(colorBuffer.texture, desc);
                desc.colorAttachments[i].resolveLevel = colorBuffer.textureLevel;
                desc.colorAttachments[i].resolveSlice = slice;
            }
            else
            {
                TextureMetal::SetAsRenderTarget(colorBuffer.texture, desc, i);
            }
        }
        else
        {
            if (i == 0)
            {
                if (cfg.UsingMSAA())
                {
                    DVASSERT(colorBuffer.multisampleTexture != InvalidHandle);
                    TextureMetal::SetAsRenderTarget(colorBuffer.multisampleTexture, desc);
                    desc.colorAttachments[0].resolveTexture = _Metal_DefFrameBuf;
                }
                else if ((cfg.depthStencilBuffer.texture == DefaultDepthBuffer) || (cfg.depthStencilBuffer.texture == InvalidHandle))
                {
                    desc.colorAttachments[i].texture = _Metal_DefFrameBuf;
                }
            }
            break;
        }
    }
}

void SetRenderPassAttachmentsExplicit(MTLRenderPassDescriptor* desc, const RenderPassConfig& cfg)
{
    if (cfg.UsingMSAA())
        abort();

    for (uint32 i = 0; i < cfg.explicitColorBuffersCount; ++i)
    {
        const RenderPassConfig::ColorBuffer& colorBuffer = cfg.colorBuffer[i];

        desc.colorAttachments[i].loadAction = GetLoadActionValue(colorBuffer.loadAction);
        desc.colorAttachments[i].storeAction = GetStoreActionValue(colorBuffer.storeAction);
        desc.colorAttachments[i].clearColor = MTLClearColorMake(colorBuffer.clearColor[0], colorBuffer.clearColor[1], colorBuffer.clearColor[2], colorBuffer.clearColor[3]);
        desc.colorAttachments[i].level = colorBuffer.textureLevel;

        int slice = 0;
        switch (colorBuffer.textureFace)
        {
        case TextureFace::TEXTURE_FACE_NONE:
        case TextureFace::TEXTURE_FACE_POSITIVE_X:
            slice = 0;
            break;
        case TextureFace::TEXTURE_FACE_NEGATIVE_X:
            slice = 1;
            break;
        case TextureFace::TEXTURE_FACE_POSITIVE_Y:
            slice = 2;
            break;
        case TextureFace::TEXTURE_FACE_NEGATIVE_Y:
            slice = 3;
            break;
        case TextureFace::TEXTURE_FACE_POSITIVE_Z:
            slice = 4;
            break;
        case TextureFace::TEXTURE_FACE_NEGATIVE_Z:
            slice = 5;
            break;
        default:
            DVASSERT(0, "Invalid texture face specified");
        }
        desc.colorAttachments[i].slice = slice;

        if (colorBuffer.texture == InvalidHandle)
        {
            desc.colorAttachments[i].texture = _Metal_DefFrameBuf;
        }
        else
        {
            TextureMetal::SetAsRenderTarget(colorBuffer.texture, desc, i);
        }
    }
}

void SetRenderPassAttachments(MTLRenderPassDescriptor* desc, const RenderPassConfig& cfg, bool& depthUsed, bool& stencilUsed)
{
    if (cfg.explicitColorBuffersCount == RenderPassConfig::ColorBuffersCountAutoDeduction)
    {
        SetRenderPassAttachmentsWithDeduction(desc, cfg);
    }
    else
    {
        SetRenderPassAttachmentsExplicit(desc, cfg);
    }

    if (cfg.depthStencilBuffer.texture == rhi::DefaultDepthBuffer)
    {
        if (cfg.UsingMSAA())
        {
            TextureMetal::SetAsDepthStencil(cfg.depthStencilBuffer.multisampleTexture, desc);
            desc.depthAttachment.resolveTexture = _Metal_DefDepthBuf;
            desc.stencilAttachment.resolveTexture = _Metal_DefStencilBuf;
        }
        else
        {
            desc.depthAttachment.texture = _Metal_DefDepthBuf;
            desc.stencilAttachment.texture = _Metal_DefStencilBuf;
        }
    }
    else if (cfg.depthStencilBuffer.texture != rhi::InvalidHandle)
    {
        if (cfg.UsingMSAA())
        {
            TextureMetal::SetAsDepthStencil(cfg.depthStencilBuffer.multisampleTexture, desc);
            TextureMetal::SetAsResolveDepthStencil(cfg.depthStencilBuffer.texture, desc);
        }
        else
        {
            TextureMetal::SetAsDepthStencil(cfg.depthStencilBuffer.texture, desc);
        }
    }

    if (desc.depthAttachment.texture != nil)
    {
        float clearDepthValue = cfg.usesReverseDepth ? (1.0f - cfg.depthStencilBuffer.clearDepth) : cfg.depthStencilBuffer.clearDepth;
        desc.depthAttachment.clearDepth = static_cast<CGFloat>(clearDepthValue); // cfg.depthStencilBuffer.clearDepth);
        desc.depthAttachment.loadAction = GetLoadActionValue(cfg.depthStencilBuffer.loadAction);
        desc.depthAttachment.storeAction = GetStoreActionValue(cfg.depthStencilBuffer.storeAction);
        depthUsed = true;
    }

    if (desc.stencilAttachment.texture != nil)
    {
        desc.stencilAttachment.clearStencil = cfg.depthStencilBuffer.clearStencil;
        desc.stencilAttachment.loadAction = GetLoadActionValue(cfg.depthStencilBuffer.loadAction);
        desc.stencilAttachment.storeAction = GetStoreActionValue(cfg.depthStencilBuffer.storeAction);
        stencilUsed = true;
    }
}

void CheckDefaultBuffers()
{
    DAVA::LockGuard<DAVA::Mutex> guard(_Metal_ResetSync);

    if (_Metal_ResetPending)
    {
        if (_Metal_DefDepthBuf)
        {
            [_Metal_DefDepthBuf release];
            _Metal_DefDepthBuf = nil;
        }

        if (_Metal_DefStencilBuf)
        {
            [_Metal_DefStencilBuf release];
            _Metal_DefStencilBuf = nil;
        }

        NSUInteger width = _Metal_ResetParam.width;
        NSUInteger height = _Metal_ResetParam.height;

        MTLTextureDescriptor* depthDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float width:width height:height mipmapped:NO];
        depthDesc.usage |= MTLTextureUsageRenderTarget;
        _Metal_DefDepthBuf = [_Metal_Device newTextureWithDescriptor:depthDesc];

        MTLTextureDescriptor* stencilDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatStencil8 width:width height:height mipmapped:NO];
        stencilDesc.usage |= MTLTextureUsageRenderTarget;
        _Metal_DefStencilBuf = [_Metal_Device newTextureWithDescriptor:stencilDesc];

        _Metal_Layer.drawableSize = CGSizeMake(width, height);

        _Metal_ResetPending = false;
    }
}

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

bool RenderPassMetal_t::Initialize()
{
    bool need_drawable = cfg.colorBuffer[0].texture == InvalidHandle;
    if (!need_drawable && cfg.explicitColorBuffersCount != RenderPassConfig::ColorBuffersCountAutoDeduction && cfg.explicitColorBuffersCount > 1)
    {
        for (uint32 i = 1; i < cfg.explicitColorBuffersCount; ++i)
        {
            if (cfg.colorBuffer[i].texture == InvalidHandle)
            {
                need_drawable = true;
                break;
            }
        }
    }
    if (need_drawable && !_Metal_currentDrawable)
    {
        if (_Metal_DrawableDispatchSemaphore != nullptr)
            _Metal_DrawableDispatchSemaphore->Wait();

        @autoreleasepool
        {
            CheckDefaultBuffers();
            _Metal_currentDrawable = [[_Metal_Layer nextDrawable] retain];
            _Metal_DefFrameBuf = _Metal_currentDrawable.texture;
        }
    }

    if (need_drawable && !_Metal_currentDrawable)
    {
        for (unsigned i = 0; i != cmdBuf.size(); ++i)
        {
            CommandBufferPoolMetal::Free(cmdBuf[i]);
        }
        cmdBuf.clear();
        return false;
    }

    bool depth_used = false;
    bool stencil_used = false;

    desc = [MTLRenderPassDescriptor renderPassDescriptor];

    SetRenderPassAttachments(desc, cfg, depth_used, stencil_used);

    if (cfg.queryBuffer != InvalidHandle)
    {
        desc.visibilityResultBuffer = QueryBufferMetal::GetBuffer(cfg.queryBuffer);
    }

#if (USE_SHARED_COMMAND_BUFFER)
    commandBuffer = [_Metal_currentCommandBuffer retain];
#else
    commandBuffer = [[_Metal_DefCmdQueue commandBuffer] retain];
#endif

    if (cmdBuf.size() == 1)
    {
        CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf[0]);

        encoder = nil;
        cb->encoder = [commandBuffer renderCommandEncoderWithDescriptor:desc];
        [cb->encoder retain];
        if (cfg.name)
            [cb->encoder setLabel:[NSString stringWithCString:cfg.name encoding:NSUTF8StringEncoding]];

        if ((cfg.depthBias != 0.0f) || (cfg.depthSlopeScale != 0.0f))
        {
            [cb->encoder setDepthBias:cfg.depthBias slopeScale:cfg.depthSlopeScale clamp:0.0f];
        }
        cb->targetLevel = desc.colorAttachments[0].level;

        cb->color_count = 0;

        if (cfg.explicitColorBuffersCount == RenderPassConfig::ColorBuffersCountAutoDeduction)
        {
            for (unsigned t = 0; t != MAX_RENDER_TARGET_COUNT; ++t)
            {
                cb->color_fmt[t] = desc.colorAttachments[t].texture.pixelFormat;

                if (cfg.colorBuffer[t].texture == InvalidHandle)
                {
                    if ((t == 0) && ((cfg.depthStencilBuffer.texture == DefaultDepthBuffer) || (cfg.depthStencilBuffer.texture == InvalidHandle)))
                    {
                        cb->color_count = 1;
                    }
                    break;
                }
                else
                {
                    ++cb->color_count;
                }
            }
        }
        else
        {
            cb->color_count = cfg.explicitColorBuffersCount;
            for (uint32 i = 0; i < cfg.explicitColorBuffersCount; ++i)
                cb->color_fmt[i] = desc.colorAttachments[i].texture.pixelFormat;
        }

        cb->rt = desc.colorAttachments[0].texture;
        if (cb->rt == nil)
        {
            cb->rt = desc.depthAttachment.texture;
        }

        cb->depth_used = depth_used;
        cb->stencil_used = stencil_used;
        cb->cur_ib = InvalidHandle;
        cb->cur_vstream_count = 0;
        cb->cur_ss = InvalidHandle;
        cb->sampleCount = rhi::TextureSampleCountForAAType(cfg.antialiasingType);
        for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
            cb->cur_vb[s] = InvalidHandle;
    }
    else
    {
        encoder = [commandBuffer parallelRenderCommandEncoderWithDescriptor:desc];
        [encoder retain];
        if (cfg.name)
            [encoder setLabel:[NSString stringWithCString:cfg.name encoding:NSUTF8StringEncoding]];

        for (unsigned i = 0; i != cmdBuf.size(); ++i)
        {
            CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf[i]);

            cb->encoder = [encoder renderCommandEncoder];
            [cb->encoder retain];
            cb->buf = commandBuffer;
            cb->rt = desc.colorAttachments[0].texture;
            cb->color_count = 0;
            for (unsigned t = 0; t != MAX_RENDER_TARGET_COUNT; ++t)
            {
                cb->color_fmt[t] = desc.colorAttachments[t].texture.pixelFormat;

                if (cfg.colorBuffer[t].texture == InvalidHandle)
                {
                    if (t == 0)
                        cb->color_count = 1;
                    break;
                }
                else
                {
                    ++cb->color_count;
                }
            }
            cb->depth_used = depth_used;
            cb->stencil_used = stencil_used;
            cb->cur_ib = InvalidHandle;
            cb->cur_vstream_count = 0;
            cb->sampleCount = rhi::TextureSampleCountForAAType(cfg.antialiasingType);
            for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
                cb->cur_vb[s] = InvalidHandle;
        }
    }

    return true;
}

#endif

//------------------------------------------------------------------------------

static Handle metal_RenderPass_Allocate(const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf)
{
    DVASSERT(passConf.IsValid());
    DVASSERT(cmdBufCount);
    

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    bool need_drawable = passConf.colorBuffer[0].texture == InvalidHandle && !_Metal_currentDrawable;
    if (!need_drawable && cfg.explicitColorBuffersCount != RenderPassConfig::ColorBuffersCountAutoDeduction && cfg.explicitColorBuffersCount > 1)
    {
        for (uint32 i = 1; i < cfg.explicitColorBuffersCount; ++i)
        {
            if (cfg.colorBuffer[i].texture == InvalidHandle)
            {
                need_drawable = true;
                break;
            }
        }
    }
    if (need_drawable)
    {
        @autoreleasepool
        {
            CheckDefaultBuffers();
            _Metal_currentDrawable = [[_Metal_Layer nextDrawable] retain];
            _Metal_DefFrameBuf = _Metal_currentDrawable.texture;
        }
    }

    if (need_drawable && !_Metal_currentDrawable)
    {
        for (unsigned i = 0; i != cmdBufCount; ++i)
            cmdBuf[i] = InvalidHandle;

        return InvalidHandle;
    }

    Handle pass_h = RenderPassPoolMetal::Alloc();
    RenderPassMetal_t* pass = RenderPassPoolMetal::Get(pass_h);

    bool depth_used = false;
    bool stencil_used = false;
    pass->desc = [MTLRenderPassDescriptor renderPassDescriptor];
    SetRenderPassAttachments(pass->desc, passConf, depth_used, stencil_used);

    if (passConf.queryBuffer != InvalidHandle)
    {
        pass->desc.visibilityResultBuffer = QueryBufferMetal::GetBuffer(passConf.queryBuffer);
    }

    pass->cmdBuf.resize(cmdBufCount);
    pass->priority = passConf.priority;
    pass->commandBuffer = [_Metal_currentCommandBuffer retain];

    if (cmdBufCount == 1)
    {
        Handle cb_h = CommandBufferPoolMetal::Alloc();
        CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cb_h);

        pass->encoder = nil;

        cb->encoder = [pass->commandBuffer renderCommandEncoderWithDescriptor:pass->desc];
        [cb->encoder retain];

        cb->color_count = 0;
        for (unsigned t = 0; t != MAX_RENDER_TARGET_COUNT; ++t)
        {
            cb->color_fmt[t] = pass->desc.colorAttachments[t].texture.pixelFormat;

            if (passConf.colorBuffer[t].texture == InvalidHandle)
            {
                if (t == 0)
                    cb->color_count = 1;
                break;
            }
            else
            {
                ++cb->color_count;
            }
        }
        cb->buf = pass->commandBuffer;
        cb->rt = pass->desc.colorAttachments[0].texture;
        cb->depth_used = depth_used;
        cb->stencil_used = stencil_used;
        cb->cur_ib = InvalidHandle;
        cb->cur_vstream_count = 0;
        cb->cur_ss = InvalidHandle;
        cb->sampleCount = rhi::TextureSampleCountForAAType(passConf.antialiasingType);
        for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
            cb->cur_vb[s] = InvalidHandle;

        pass->cmdBuf[0] = cb_h;
        cmdBuf[0] = cb_h;
    }
    else
    {
        pass->encoder = [pass->commandBuffer parallelRenderCommandEncoderWithDescriptor:pass->desc];
        [pass->encoder retain];

        for (unsigned i = 0; i != cmdBufCount; ++i)
        {
            Handle cb_h = CommandBufferPoolMetal::Alloc();
            CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cb_h);

            cb->encoder = [pass->encoder renderCommandEncoder];
            [cb->encoder retain];
            cb->color_count = 0;
            for (unsigned t = 0; t != MAX_RENDER_TARGET_COUNT; ++t)
            {
                cb->color_fmt[t] = pass->desc.colorAttachments[t].texture.pixelFormat;

                if (passConf.colorBuffer[t].texture == InvalidHandle)
                {
                    if (t == 0)
                        cb->color_count = 1;
                    break;
                }
                else
                {
                    ++cb->color_count;
                }
            }

            cb->buf = pass->commandBuffer;
            cb->rt = pass->desc.colorAttachments[0].texture;
            cb->depth_used = depth_used;
            cb->stencil_used = stencil_used;
            cb->cur_ib = InvalidHandle;
            cb->cur_vstream_count = 0;
            cb->sampleCount = rhi::TextureSampleCountForAAType(passConf.antialiasingType);
            for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
                cb->cur_vb[s] = InvalidHandle;

            pass->cmdBuf[i] = cb_h;
            cmdBuf[i] = cb_h;
        }
    }

    return pass_h;
    
#else

    Handle pass_h = RenderPassPoolMetal::Alloc();
    RenderPassMetal_t* pass = RenderPassPoolMetal::Get(pass_h);

    pass->cfg = passConf;
    pass->priority = passConf.priority;

    pass->cmdBuf.resize(cmdBufCount);
    for (unsigned i = 0; i != cmdBufCount; ++i)
    {
        Handle cb_h = CommandBufferPoolMetal::Alloc();
        CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cb_h);

        bool hasDepthTexture = (passConf.depthStencilBuffer.texture != rhi::InvalidHandle);
        bool hasDefaultDepth = passConf.depthStencilBuffer.texture == DefaultDepthBuffer;
        if (hasDepthTexture)
        {
            cb->depth_used = true;
            cb->stencil_used = hasDefaultDepth || (TextureMetal::GetFormat(passConf.depthStencilBuffer.texture) == TEXTURE_FORMAT_D24S8);
        }

        pass->cmdBuf[i] = cb_h;
        cmdBuf[i] = cb_h;
    }

    return pass_h;

#endif
}

static void metal_RenderPass_Begin(Handle pass_h)
{
    RenderPassMetal_t* pass = RenderPassPoolMetal::Get(pass_h);
}

static void metal_RenderPass_End(Handle pass_h)
{
    RenderPassMetal_t* pass = RenderPassPoolMetal::Get(pass_h);

    #if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    if (pass->cmdBuf.size() > 1)
    {
        [pass->encoder endEncoding];
    }
    #endif
}

namespace RenderPassMetal
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_Renderpass_Allocate = &metal_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin = &metal_RenderPass_Begin;
    dispatch->impl_Renderpass_End = &metal_RenderPass_End;
}
}

//------------------------------------------------------------------------------

static void metal_CommandBuffer_Begin(Handle cmdBuf)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

    cb->cur_vstream_count = 0;
    for (unsigned s = 0; s != countof(cb->cur_vb); ++s)
        cb->cur_vb[s] = InvalidHandle;

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    [cb->encoder setDepthStencilState:_Metal_DefDepthState];
#else
    cb->curUsedSize = 0;
    cb->allocCmd<SWCommand_Begin>();
#endif
}

//------------------------------------------------------------------------------

static void metal_CommandBuffer_End(Handle cmdBuf, Handle syncObject)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    [cb->encoder endEncoding];

    if (syncObject != InvalidHandle)
    {
        [cb->buf addCompletedHandler:^(id<MTLCommandBuffer> cmdb) {
          SyncObjectMetal_t* sync = SyncObjectPoolMetal::Get(syncObject);

          sync->is_signaled = true;
        }];
    }
    
#else
    SWCommand_End* cmd = cb->allocCmd<SWCommand_End>();
    cmd->syncObject = syncObject;
#endif
}

//------------------------------------------------------------------------------

static void metal_CommandBuffer_SetPipelineState(Handle cmdBuf, Handle ps, uint32 layoutUID)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    cb->cur_stride = PipelineStateMetal::SetToRHI(ps, layoutUID, cb->color_fmt, cb->color_count, cb->depth_used, cb->stencil_used, cb->encoder, cb->sampleCount);
    cb->cur_vstream_count = PipelineStateMetal::VertexStreamCount(ps);
    StatSet::IncStat(stat_SET_PS, 1);
#else
    SWCommand_SetPipelineState* cmd = cb->allocCmd<SWCommand_SetPipelineState>();
    cmd->ps = ps;
    cmd->vdecl = layoutUID;
#endif
}

//------------------------------------------------------------------------------

static void metal_CommandBuffer_SetCullMode(Handle cmdBuf, CullMode mode)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    switch (mode)
    {
    case CULL_NONE:
        [cb->encoder setCullMode:MTLCullModeNone];
        break;

    case CULL_CCW:
        [cb->encoder setFrontFacingWinding:MTLWindingClockwise];
        [cb->encoder setCullMode:MTLCullModeBack];
        break;

    case CULL_CW:
        [cb->encoder setFrontFacingWinding:MTLWindingClockwise];
        [cb->encoder setCullMode:MTLCullModeFront];
        break;
    }
#else
    SWCommand_SetCullMode* cmd = cb->allocCmd<SWCommand_SetCullMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

static void metal_CommandBuffer_SetScissorRect(Handle cmdBuf, ScissorRect rect)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    MTLScissorRect rc;
    if (!(rect.x == 0 && rect.y == 0 && rect.width == 0 && rect.height == 0))
    {
        unsigned max_x = (cb->rt) ? unsigned(cb->rt.width >> cb->targetLevel) : unsigned(_Metal_DefFrameBuf.width);
        unsigned max_y = (cb->rt) ? unsigned(cb->rt.height >> cb->targetLevel) : unsigned(_Metal_DefFrameBuf.height);

        rc.x = rect.x;
        rc.y = rect.y;
        rc.width = (rect.x + rect.width > max_x) ? (max_x - rc.x) : rect.width;
        rc.height = (rect.y + rect.height > max_y) ? (max_y - rc.y) : rect.height;

        if (rc.width == 0)
        {
            rc.width = 1;
            if (rc.x > 0)
                --rc.x;
        }

        if (rc.height == 0)
        {
            rc.height = 1;
            if (rc.y > 0)
                --rc.y;
        }
    }
    else
    {
        rc.x = 0;
        rc.y = 0;
        if (cb->rt)
        {
            rc.width = cb->rt.width >> cb->targetLevel;
            rc.height = cb->rt.height >> cb->targetLevel;
        }
        else
        {
            rc.width = _Metal_DefFrameBuf.width;
            rc.height = _Metal_DefFrameBuf.height;
        }
    }

    [cb->encoder setScissorRect:rc];

#else

    int x = rect.x;
    int y = rect.y;
    int w = rect.width;
    int h = rect.height;
    SWCommand_SetScissorRect* cmd = cb->allocCmd<SWCommand_SetScissorRect>();
    cmd->x = x;
    cmd->y = y;
    cmd->width = w;
    cmd->height = h;
    
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetViewport(Handle cmdBuf, Viewport viewport)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    MTLViewport vp;
    if (!(viewport.x == 0 && viewport.y == 0 && viewport.width == 0 && viewport.height == 0))
    {
        vp.originX = viewport.x;
        vp.originY = viewport.y;
        vp.width = viewport.width;
        vp.height = viewport.height;
        vp.znear = 0.0;
        vp.zfar = 1.0;
    }
    else
    {
        vp.originX = 0;
        vp.originY = 0;
        vp.width = cb->rt.width >> cb->targetLevel;
        vp.height = cb->rt.height >> cb->targetLevel;
        vp.znear = 0.0;
        vp.zfar = 1.0;
    }

    [cb->encoder setViewport:vp];

#else

    int x = viewport.x;
    int y = viewport.y;
    int w = viewport.width;
    int h = viewport.height;
    SWCommand_SetViewport* cmd = cb->allocCmd<SWCommand_SetViewport>();
    cmd->x = x;
    cmd->y = y;
    cmd->width = w;
    cmd->height = h;
    

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFillMode(Handle cmdBuf, FillMode mode)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    [cb->encoder setTriangleFillMode:(mode == FILLMODE_WIREFRAME) ? MTLTriangleFillModeLines : MTLTriangleFillModeFill];
#else
    SWCommand_SetFillMode* cmd = cb->allocCmd<SWCommand_SetFillMode>();
    cmd->mode = mode;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexData(Handle cmdBuf, Handle vb, uint32 streamIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    cb->cur_vb[streamIndex] = vb;
    StatSet::IncStat(stat_SET_VB, 1);
#else
    SWCommand_SetVertexData* cmd = cb->allocCmd<SWCommand_SetVertexData>();
    cmd->vb = vb;
    cmd->streamIndex = streamIndex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    ConstBufferMetal::SetToRHI(buffer, bufIndex, cb->encoder);
    StatSet::IncStat(stat_SET_CB, 1);
#else
    SWCommand_SetVertexProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetVertexProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    uintptr_t instOffset = static_cast<uintptr_t>(ConstBufferMetal::Instance(buffer));
    cmd->inst = reinterpret_cast<void*>(instOffset);
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetVertexTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    TextureMetal::SetToRHIVertex(tex, unitIndex, cb->encoder);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    SWCommand_SetVertexTexture* cmd = cb->allocCmd<SWCommand_SetVertexTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetIndices(Handle cmdBuf, Handle ib)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    cb->cur_ib = ib;
    StatSet::IncStat(stat_SET_IB, 1);
#else
    SWCommand_SetIndices* cmd = cb->allocCmd<SWCommand_SetIndices>();
    cmd->ib = ib;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryIndex(Handle cmdBuf, uint32 objectIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    if (objectIndex != DAVA::InvalidIndex)
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:objectIndex * QueryBUfferElemeentAlign];
    }
    else
    {
        [cb->encoder setVisibilityResultMode:MTLVisibilityResultModeDisabled offset:0];
    }
#else
    SWCommand_SetQueryIndex* cmd = cb->allocCmd<SWCommand_SetQueryIndex>();
    cmd->objectIndex = objectIndex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetQueryBuffer(Handle /*cmdBuf*/, Handle /*queryBuf*/)
{
    // do NOTHING, since query-buffer specified in render-pass
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentConstBuffer(Handle cmdBuf, uint32 bufIndex, Handle buffer)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    ConstBufferMetal::SetToRHI(buffer, bufIndex, cb->encoder);
    StatSet::IncStat(stat_SET_CB, 1);
#else
    SWCommand_SetFragmentProgConstBuffer* cmd = cb->allocCmd<SWCommand_SetFragmentProgConstBuffer>();
    cmd->bufIndex = bufIndex;
    cmd->buffer = buffer;
    uintptr_t instOffset = static_cast<uintptr_t>(ConstBufferMetal::Instance(buffer));
    cmd->inst = reinterpret_cast<void*>(instOffset);
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetFragmentTexture(Handle cmdBuf, uint32 unitIndex, Handle tex)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);
    
#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    TextureMetal::SetToRHIFragment(tex, unitIndex, cb->encoder);
    StatSet::IncStat(stat_SET_TEX, 1);
#else
    SWCommand_SetFragmentTexture* cmd = cb->allocCmd<SWCommand_SetFragmentTexture>();
    cmd->unitIndex = unitIndex;
    cmd->tex = tex;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetDepthStencilState(Handle cmdBuf, Handle depthStencilState)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    DepthStencilStateMetal::SetToRHI(depthStencilState, cb->encoder);
#else
    SWCommand_SetDepthStencilState* cmd = cb->allocCmd<SWCommand_SetDepthStencilState>();
    cmd->depthStencilState = depthStencilState;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_SetSamplerState(Handle cmdBuf, const Handle samplerState)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    SamplerStateMetal::SetToRHI(samplerState, cb->encoder);
    StatSet::IncStat(stat_SET_SS, 1);
#else
    SWCommand_SetSamplerState* cmd = cb->allocCmd<SWCommand_SetSamplerState>();
    cmd->samplerState = samplerState;
#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned v_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        v_cnt = count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        v_cnt = 2 + count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        v_cnt = count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    cb->_ApplyVertexData();
    [cb->encoder drawPrimitives:ptype vertexStart:0 vertexCount:v_cnt];

    StatSet::IncStat(stat_DP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }

#else

    SWCommand_DrawPrimitive* cmd = cb->allocCmd<SWCommand_DrawPrimitive>();

    cmd->mode = ptype;
    cmd->vertexCount = v_cnt;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned i_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        i_cnt = count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        i_cnt = 2 + count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        i_cnt = count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    unsigned ib_base = 0;
    id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cb->cur_ib, &ib_base);
    MTLIndexType i_type = IndexBufferMetal::GetType(cb->cur_ib);
    unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

    cb->_ApplyVertexData(firstVertex);
    [cb->encoder drawIndexedPrimitives:ptype indexCount:i_cnt indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off];

    StatSet::IncStat(stat_DIP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }

#else

    SWCommand_DrawIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawIndexedPrimitive>();

    cmd->mode = ptype;
    cmd->indexCount = i_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawInstancedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 inst_count, uint32 prim_count)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned v_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        v_cnt = prim_count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        v_cnt = 2 + prim_count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        v_cnt = prim_count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    cb->_ApplyVertexData();
    [cb->encoder drawPrimitives:ptype vertexStart:0 vertexCount:v_cnt instanceCount:inst_count];

    StatSet::IncStat(stat_DP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }

#else

    SWCommand_DrawInstancedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedPrimitive>();

    cmd->mode = ptype;
    cmd->instanceCount = inst_count;
    cmd->vertexCount = v_cnt;

#endif
}

//------------------------------------------------------------------------------

static void
metal_CommandBuffer_DrawInstancedIndexedPrimitive(Handle cmdBuf, PrimitiveType type, uint32 instCount, uint32 prim_count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex, uint32 baseInst)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);
    MTLPrimitiveType ptype = MTLPrimitiveTypeTriangle;
    unsigned i_cnt = 0;

    switch (type)
    {
    case PRIMITIVE_TRIANGLELIST:
        ptype = MTLPrimitiveTypeTriangle;
        i_cnt = prim_count * 3;
        break;

    case PRIMITIVE_TRIANGLESTRIP:
        ptype = MTLPrimitiveTypeTriangleStrip;
        i_cnt = 2 + prim_count;
        break;

    case PRIMITIVE_LINELIST:
        ptype = MTLPrimitiveTypeLine;
        i_cnt = prim_count * 2;
        break;
    }

#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    unsigned ib_base = 0;
    id<MTLBuffer> ib = IndexBufferMetal::GetBuffer(cb->cur_ib, &ib_base);
    MTLIndexType i_type = IndexBufferMetal::GetType(cb->cur_ib);
    unsigned i_off = (i_type == MTLIndexTypeUInt16) ? startIndex * sizeof(uint16) : startIndex * sizeof(uint32);

    cb->_ApplyVertexData(firstVertex);
    [cb->encoder drawIndexedPrimitives:ptype indexCount:i_cnt indexType:i_type indexBuffer:ib indexBufferOffset:ib_base + i_off instanceCount:instCount];

    StatSet::IncStat(stat_DIP, 1);
    switch (ptype)
    {
    case MTLPrimitiveTypeTriangle:
        StatSet::IncStat(stat_DTL, 1);
        break;
    case MTLPrimitiveTypeTriangleStrip:
        StatSet::IncStat(stat_DTS, 1);
        break;
    case MTLPrimitiveTypeLine:
        StatSet::IncStat(stat_DLL, 1);
        break;
    default:
        break;
    }
#else
    SWCommand_DrawInstancedIndexedPrimitive* cmd = cb->allocCmd<SWCommand_DrawInstancedIndexedPrimitive>();

    cmd->mode = ptype;
    cmd->indexCount = i_cnt;
    cmd->firstVertex = firstVertex;
    cmd->startIndex = startIndex;
    cmd->instanceCount = instCount;
    cmd->baseInstance = baseInst;

#endif
}

//------------------------------------------------------------------------------

static void metal_CommandBuffer_SetMarker(Handle cmdBuf, const char* text)
{
    CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cmdBuf);
    
#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
    NSString* txt = [[NSString alloc] initWithUTF8String:text];
    [cb->encoder insertDebugSignpost:txt];
    [txt release];
#else
    SWCommand_SetMarker* cmd = cb->allocCmd<SWCommand_SetMarker>();
    cmd->text = text;
#endif
}

//------------------------------------------------------------------------------

static Handle
metal_SyncObject_Create()
{
    DAVA::LockGuard<DAVA::Mutex> guard(_Metal_SyncObjectsSync);
    Handle handle = SyncObjectPoolMetal::Alloc();
    SyncObjectMetal_t* sync = SyncObjectPoolMetal::Get(handle);

    sync->is_signaled = false;

    return handle;
}

//------------------------------------------------------------------------------

static void
metal_SyncObject_Delete(Handle obj)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_Metal_SyncObjectsSync);
    SyncObjectPoolMetal::Free(obj);
}

//------------------------------------------------------------------------------

static bool
metal_SyncObject_IsSignaled(Handle obj)
{
    bool signaled = false;
    DAVA::LockGuard<DAVA::Mutex> guard(_Metal_SyncObjectsSync);
    if (SyncObjectPoolMetal::IsAlive(obj))
    {
        SyncObjectMetal_t* sync = SyncObjectPoolMetal::Get(obj);

        if (sync)
            signaled = sync->is_signaled;
    }
    else
    {
        signaled = true;
    }

    return signaled;
}

static void Metal_RejectFrame(const CommonImpl::Frame& frame)
{
#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS

    for (unsigned i = 0; i != frame.pass.size(); ++i)
    {
        if (frame.pass[i] == InvalidHandle)
            continue;
        RenderPassMetal_t* rp = RenderPassPoolMetal::Get(frame.pass[i]);

        for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
        {
            Handle cbh = rp->cmdBuf[b];
            CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cbh);

            cb->buf = nil;
            [cb->encoder release];
            cb->encoder = nil;
            cb->rt = nil;
        }

        rp->desc = nullptr;

        [rp->commandBuffer release];
        rp->commandBuffer = nil;

        [rp->encoder release];
        rp->encoder = nil;
    }
#endif

    @autoreleasepool
    {
        for (unsigned i = 0; i != frame.pass.size(); ++i)
        {
            if (frame.pass[i] == InvalidHandle)
                continue;
            RenderPassMetal_t* rp = RenderPassPoolMetal::Get(frame.pass[i]);

            for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
            {
                Handle cbh = rp->cmdBuf[b];
                CommandBufferPoolMetal::Free(cbh);
            }
            rp->cmdBuf.clear();
            RenderPassPoolMetal::Free(frame.pass[i]);
        }

        if (_Metal_currentDrawable != nil)
        {
            [_Metal_currentDrawable release];
            _Metal_currentDrawable = nil;
            _Metal_DefFrameBuf = nil;
        }

        if (frame.sync != InvalidHandle)
        {
            DAVA::LockGuard<DAVA::Mutex> guard(_Metal_SyncObjectsSync);
            SyncObjectMetal_t* sync = SyncObjectPoolMetal::Get(frame.sync);
            sync->is_signaled = true;
        }
    }
}

//------------------------------------------------------------------------------

static void Metal_ExecuteQueuedCommands(const CommonImpl::Frame& frame)
{
    @autoreleasepool
    {
//for some reason when device is locked receive nil in drawable even before getting notification - check this case and do nothing to prevent crashes
#if RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
        if (_Metal_currentDrawable == nil)
        {
            Metal_RejectFrame(frame);
            return;
        }
#endif

#if (USE_SHARED_COMMAND_BUFFER)
        _Metal_currentCommandBuffer = [[_Metal_DefCmdQueue commandBuffer] retain];
#endif

        // sort cmd-lists by priority - command buffer are to be committed in pass-priority order
        static std::vector<RenderPassMetal_t*> pass;
        Handle syncObject = frame.sync;
        pass.clear();
        for (unsigned i = 0; i != frame.pass.size(); ++i)
        {
            RenderPassMetal_t* rp = RenderPassPoolMetal::Get(frame.pass[i]);
            bool do_add = true;

            for (std::vector<RenderPassMetal_t *>::iterator p = pass.begin(), p_end = pass.end(); p != p_end; ++p)
            {
                if (rp->priority > (*p)->priority)
                {
                    pass.insert(p, 1, rp);
                    do_add = false;
                    break;
                }
            }

            if (do_add)
                pass.push_back(rp);
        }

        // commit everything here - software command buffer are executed priorly
        // also add completion handlers here as befor rp->Initialize we dont have command buffers / frame drawable, and after committing adding handlers is prohibited
        bool initOk = true;
        for (int32 i = 0, sz = pass.size(); i < sz; ++i)
        {
            RenderPassMetal_t* rp = pass[i];

#if !RHI_METAL__USE_NATIVE_COMMAND_BUFFERS
            // execute software command buffers
            initOk &= rp->Initialize();
            if (!initOk)
                break;
            for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
            {
                Handle cbh = rp->cmdBuf[b];
                CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cbh);
                cb->Execute();
            }
            if (rp->encoder != nil)
                [rp->encoder endEncoding];
#endif

            if (i == (sz - 1))
            {
                //present drawable adds completion handler that calls actual present
                [rp->commandBuffer presentDrawable:_Metal_currentDrawable];
                unsigned frame_n = frame.frameNumber;
                if (syncObject != InvalidHandle)
                {
                    [rp->commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb)
                                                           {
                                                             DAVA::LockGuard<DAVA::Mutex> guard(_Metal_SyncObjectsSync);
                                                             SyncObjectMetal_t* sync = SyncObjectPoolMetal::Get(syncObject);
                                                             sync->is_signaled = true;
                                                           }];
                }
                if (_Metal_DrawableDispatchSemaphore != nullptr)
                {
                    [rp->commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> cb)
                                                           {
                                                             _Metal_DrawableDispatchSemaphore->Post();
                                                           }];
                }
            }

        #if (!USE_SHARED_COMMAND_BUFFER)
            [rp->commandBuffer commit];
        #endif
        }

        if (!initOk)
        {
            //for some reason when device is locked receive nil in drawable even before getting notification - check this case and do nothing to prevent crashes
            Metal_RejectFrame(frame);
            return;
        }

    #if (USE_SHARED_COMMAND_BUFFER)
        [_Metal_currentCommandBuffer commit];
    #endif

        //clear passes
        for (RenderPassMetal_t* rp : pass)
        {
            for (unsigned b = 0; b != rp->cmdBuf.size(); ++b)
            {
                Handle cbh = rp->cmdBuf[b];
                CommandBufferMetal_t* cb = CommandBufferPoolMetal::Get(cbh);

                cb->buf = nil;
                [cb->encoder release];
                cb->encoder = nil;
                cb->rt = nil;

                CommandBufferPoolMetal::Free(cbh);
            }

            rp->desc = nullptr;

            [rp->commandBuffer release];
            rp->commandBuffer = nil;
            if (rp->encoder != nil)
            {
                [rp->encoder release];
                rp->encoder = nil;
            }

            rp->cmdBuf.clear();
        }

        for (Handle p : frame.pass)
            RenderPassPoolMetal::Free(p);

//release frame stuff
#if (USE_SHARED_COMMAND_BUFFER)
        [_Metal_currentCommandBuffer release];
        _Metal_currentCommandBuffer = nil;
#endif

        [_Metal_currentDrawable release];
        _Metal_currentDrawable = nil;
        _Metal_DefFrameBuf = nil;
    }
}

static bool Metal_PresentBuffer()
{
    return true;
}

static void Metal_InvalidateFrameCache()
{
    ConstBufferMetal::InvalidateAllInstances();
    ConstBufferMetal::ResetRingBuffer();
}

static void Metal_Suspend()
{
    Logger::Debug(" ***** Metal_Suspend");
}

static void metal_Reset(const ResetParam& param)
{
    DAVA::LockGuard<DAVA::Mutex> guard(_Metal_ResetSync);

    _Metal_ResetParam = param;
    _Metal_ResetPending = true;
}

namespace CommandBufferMetal
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_CommandBuffer_Begin = &metal_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End = &metal_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState = &metal_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode = &metal_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect = &metal_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport = &metal_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetFillMode = &metal_CommandBuffer_SetFillMode;
    dispatch->impl_CommandBuffer_SetVertexData = &metal_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer = &metal_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture = &metal_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices = &metal_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetQueryBuffer = &metal_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex = &metal_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &metal_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture = &metal_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState = &metal_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState = &metal_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive = &metal_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive = &metal_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_DrawInstancedPrimitive = &metal_CommandBuffer_DrawInstancedPrimitive;
    dispatch->impl_CommandBuffer_DrawInstancedIndexedPrimitive = &metal_CommandBuffer_DrawInstancedIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker = &metal_CommandBuffer_SetMarker;

    dispatch->impl_SyncObject_Create = &metal_SyncObject_Create;
    dispatch->impl_SyncObject_Delete = &metal_SyncObject_Delete;
    dispatch->impl_SyncObject_IsSignaled = &metal_SyncObject_IsSignaled;

    dispatch->impl_ExecuteFrame = &Metal_ExecuteQueuedCommands;
    dispatch->impl_RejectFrame = &Metal_RejectFrame;
    dispatch->impl_PresentBuffer = &Metal_PresentBuffer;
    dispatch->impl_FinishFrame = &Metal_InvalidateFrameCache;
    dispatch->impl_FinishRendering = &Metal_Suspend;
    dispatch->impl_Reset = &metal_Reset;
}
}

} // namespace rhi

#endif //#if !(TARGET_IPHONE_SIMULATOR==1)
