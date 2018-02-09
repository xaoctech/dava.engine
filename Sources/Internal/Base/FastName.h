#ifndef __DAVAENGINE_FAST_NAME__
#define __DAVAENGINE_FAST_NAME__

#include "Base/Hash.h"
#include "Base/Any.h"
#include "Concurrency/Spinlock.h"

namespace DAVA
{
struct FastNameView
{
    using CharT = char;

    FastNameView() = default;
    FastNameView(const CharT* str_, size_t size_);
    bool operator==(const FastNameView& fview) const;

    const CharT* str = nullptr;
    size_t size = 0;
};

struct FastNameViewHash
{
    size_t operator()(const DAVA::FastNameView& f) const;
};

class FastNameDB final
{
    friend class FastName;

public:
    using MutexT = Spinlock;

    static FastNameDB* GetLocalDB();
    void SetMasterDB(FastNameDB* masterDB);

private:
    FastNameDB();
    ~FastNameDB();

    static FastNameDB** GetLocalDBPtr();

    Vector<FastNameView> namesTable;
    UnorderedMap<FastNameView, size_t, FastNameViewHash> nameToIndexMap;

    MutexT mutex;
    size_t sizeOfNames = 0;
};

class FastName
{
public:
    FastName() = default;
    FastName(const char* name, size_t len);

    explicit FastName(const char* name);
    explicit FastName(const String& name);

    bool operator<(const FastName& _name) const;
    bool operator==(const FastName& _name) const;
    bool operator!=(const FastName& _name) const;

    bool empty() const;
    const char* c_str() const;
    size_t size() const;
    size_t find(const char* s, size_t pos = 0) const;
    size_t find(const String& str, size_t pos = 0) const;
    size_t find(const FastName& fn, size_t pos = 0) const;

    bool IsValid() const;

private:
    void Init(FastNameView fview);
    FastNameView view;
};

inline FastNameView::FastNameView(const FastNameView::CharT* str_, size_t size_)
    : str(str_)
    , size(size_)
{
}

inline bool FastNameView::operator==(const FastNameView& fview) const
{
    return (size == fview.size) && (::strncmp(str, fview.str, size) == 0);
}

inline FastName::FastName(const String& name)
{
    Init(FastNameView(name.c_str(), name.size()));
}

inline FastName::FastName(const char* name)
{
    Init(FastNameView(name, ::strlen(name)));
}

inline FastName::FastName(const char* name, size_t len)
{
    Init(FastNameView(name, len));
}

inline bool FastName::operator==(const FastName& fname) const
{
    return view.str == fname.view.str;
}

inline bool FastName::operator!=(const FastName& fname) const
{
    return view.str != fname.view.str;
}

inline bool FastName::operator<(const FastName& fname) const
{
    return view.str < fname.view.str;
}

inline bool FastName::empty() const
{
    return view.str == nullptr;
}

inline size_t FastName::size() const
{
    return view.size;
}

inline size_t FastName::find(const char* s, size_t pos) const
{
    const char* str = view.str;
    if (nullptr != str && nullptr != s)
    {
        DVASSERT(pos <= strlen(str));

        const char* q = strstr(str + pos, s);
        return q ? (q - str) : String::npos;
    }
    return String::npos;
}

inline size_t FastName::find(const String& str, size_t pos) const
{
    return find(str.c_str(), pos);
}

inline size_t FastName::find(const FastName& fn, size_t pos) const
{
    return find(fn.c_str(), pos);
}

inline bool FastName::IsValid() const
{
    return !empty();
}

inline const char* FastName::c_str() const
{
    return view.str;
}

template <>
bool AnyCompare<FastName>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<FastName>;
};

namespace std
{
template <>
struct hash<DAVA::FastName>
{
    size_t operator()(const DAVA::FastName& f) const
    {
        return reinterpret_cast<size_t>(f.c_str());
    }
};
}

#endif // __DAVAENGINE_FAST_NAME__
