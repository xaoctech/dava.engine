#pragma once

#include "../Common/rhi_Private.h"

namespace rhi
{
struct GLCommand;

class ProgGLES2
{
public:
    ProgGLES2(ProgType t);
    ~ProgGLES2() = default;

    bool Construct(const char* src_data);
    void Destroy();

    uint32 ShaderUid();
    void GetProgParams(uint32 progUid);
    uint32 SamplerCount();

    uint32 ConstBufferCount();
    Handle InstanceConstBuffer(uint32 bufIndex);

    void SetupTextureUnits(uint32 baseUnit, GLCommand* commands, uint32& commandsCount);

    static void InvalidateAllConstBufferInstances();

    class ConstBuf
    {
    public:
        struct Desc
        {
        };

        ConstBuf()
            : frame(static_cast<uint32>(CurFrame - 1))
        {
        }

        ~ConstBuf()
        {
            ConstBuf::Destroy();
        }

        bool Construct(uint32 prog, float** lastBoundData, uint32 loc, uint32 count);
        void Destroy();

        uint32 ConstCount();
        bool SetConst(uint32 const_i, uint32 count, const float* cdata);
        bool SetConst(uint32 const_i, uint32 const_sub_i, const float* cdata, uint32 data_count);

        const void* Instance();
        void SetToRHI(uint32 progUid, const float* instData);

        static void AdvanceFrame();

    private:
        void ReallocIfneeded();

        uint32 glProg = 0;
        uint32 location = uint32(-1);
        uint32 count = 0;
        uint32 dataSize = 0;
        uint32 frame = 0;
        float* inst = nullptr;
        float* data = nullptr;
        float** lastInst = nullptr;

        friend class ProgGLES2;
        static uint32 CurFrame;
    };

private:
    struct ConstBufInfo
    {
        uint32 location = DAVA::InvalidIndex;
        uint32 count = 0;
    };

    ConstBufInfo cbuf[MAX_CONST_BUFFER_COUNT];
    float* cbufLastBoundData[MAX_CONST_BUFFER_COUNT]{};

    uint32 texunitLoc[16]{};

    uint32 shader = 0;
    uint32 prog = 0;
    ProgType type = ProgType::PROG_VERTEX;
    uint32 texunitCount = 0;
    bool texunitInited = false;
};

} // namespace rhi
