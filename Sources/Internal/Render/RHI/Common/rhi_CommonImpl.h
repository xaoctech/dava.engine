#pragma once
#include "../rhi_Type.h"

namespace rhi
{
namespace CommonImpl
{
struct Frame
{
    Handle sync = InvalidHandle;
    Handle perfQueryStart = InvalidHandle;
    Handle perfQueryEnd = InvalidHandle;
    std::vector<Handle> pass;
    uint32 frameNumber = 0;
    bool readyToExecute = false;
    bool discarded = false;

    void Reset();
};

struct ImmediateCommand
{
    void* cmdData = nullptr; //TODO - should be common immediate command interface like software command ?
    uint32 cmdCount = 0;
    bool forceExecute = false;
};

struct TextureSet_t
{
    struct Desc
    {
    };

    uint32 fragmentTextureCount = 0;
    Handle fragmentTexture[MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT];
    uint32 vertexTextureCount = 0;
    Handle vertexTexture[MAX_VERTEX_TEXTURE_SAMPLER_COUNT];
    uint32 textureDynamic = 0;
    int refCount = 0;

    TextureSet_t()
    {
        memset(fragmentTexture, 0, sizeof(fragmentTexture));
        memset(vertexTexture, 0, sizeof(vertexTexture));
    }
};
}

namespace TextureSet
{
Handle Create();
CommonImpl::TextureSet_t* Get(Handle);
void Delete(Handle);
void InitTextreSetPool(uint32 maxCount);
}
}
