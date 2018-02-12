    #include "../rhi_ShaderCache.h"
    #include "../rhi_ShaderSource.h"

namespace rhi
{
static ShaderBuilder _ShaderBuilder = nullptr;

struct ProgInfo
{
    DAVA::FastName uid;
    std::vector<uint8> bin;
};

static std::vector<ProgInfo> _ProgInfo;

namespace ShaderCache
{
//------------------------------------------------------------------------------

bool Initialize(ShaderBuilder builder)
{
    _ShaderBuilder = builder;
    return true;
}

//------------------------------------------------------------------------------

void Unitialize()
{
}

//------------------------------------------------------------------------------

void Clear()
{
    _ProgInfo.clear();
}

//------------------------------------------------------------------------------

void Load(const char* binFileName)
{
}

//------------------------------------------------------------------------------

const std::vector<uint8>& GetProg(const DAVA::FastName& uid)
{
    static const std::vector<uint8> empty(0);

    for (unsigned i = 0; i != _ProgInfo.size(); ++i)
    {
        if (_ProgInfo[i].uid == uid)
        {
            return _ProgInfo[i].bin;
        }
    }

    return empty;
}

void UpdateProg(Api targetApi, ProgType progType, const DAVA::FastName& uid, const char* srcText)
{
    ShaderSource src;

    if (src.Construct(progType, srcText))
    {
        const std::string& code = src.GetSourceCode(targetApi);

        UpdateProgBinary(targetApi, progType, uid, code.c_str(), unsigned(code.length()));
        //DAVA::Logger::Info("\n\n--shader  \"%s\"", uid.c_str());
        //DAVA::Logger::Info(code.c_str());
    }
}

void UpdateProgBinary(Api targetApi, ProgType progType, const DAVA::FastName& uid, const void* bin, unsigned binSize)
{
    std::vector<uint8>* pbin = nullptr;

    for (unsigned i = 0; i != _ProgInfo.size(); ++i)
    {
        if (_ProgInfo[i].uid == uid)
        {
            pbin = &(_ProgInfo[i].bin);
            break;
        }
    }

    if (!pbin)
    {
        _ProgInfo.push_back(ProgInfo());

        _ProgInfo.back().uid = uid;
        pbin = &(_ProgInfo.back().bin);
    }

    //- DAVA::Logger::Info("\n\n--shader  \"%s\"", uid.c_str());
    //- DAVA::Logger::Info((const char*)bin);
    pbin->clear();
    pbin->insert(pbin->begin(), reinterpret_cast<const uint8*>(bin), reinterpret_cast<const uint8*>(bin) + binSize);
    pbin->push_back(0);
}

} // namespace ShaderCache
} // namespace rhi
