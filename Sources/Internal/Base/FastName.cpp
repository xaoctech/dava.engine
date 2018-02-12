#include "FastName.h"
#include "Debug/DVAssert.h"

#include "Concurrency/LockGuard.h"

namespace DAVA
{
size_t FastNameViewHash::operator()(const DAVA::FastNameView& f) const
{
    size_t ret = 0;
    for (size_t i = 0; i < f.size; ++i)
    {
        ret = 5 * ret + f.str[i];
    }
    return ret;
}

FastNameDB::FastNameDB()
    : nameToIndexMap(8192 * 2)
{
    namesTable.reserve(512);
}

FastNameDB::~FastNameDB()
{
    const size_t count = namesTable.size();
    for (size_t i = 0; i < count; ++i)
    {
        SafeDeleteArray(namesTable[i].str);
    }
}

FastNameDB* FastNameDB::GetLocalDB()
{
    return *GetLocalDBPtr();
}

FastNameDB** FastNameDB::GetLocalDBPtr()
{
    static FastNameDB db;
    static FastNameDB* dbPtr = &db;
    return &dbPtr;
}

void FastNameDB::SetMasterDB(FastNameDB* db)
{
    static bool hasMaster = false;

    DVASSERT(!hasMaster);
    hasMaster = true;

    // Override db from local to master
    FastNameDB** localDBPtr = GetLocalDBPtr();
    *localDBPtr = db;
}

void FastName::Init(FastNameView fview)
{
    DVASSERT(nullptr != fview.str);

    FastNameDB* db = FastNameDB::GetLocalDB();
    LockGuard<FastNameDB::MutexT> guard(db->mutex);

    // search if that name is already in hash
    auto it = db->nameToIndexMap.find(fview);
    if (it != db->nameToIndexMap.end())
    {
        // already exist, so we just need to set the same index to this object
        view = db->namesTable[it->second];
    }
    else
    {
        // string isn't in hash and it isn't in names table, so we need to copy it
        // and find place for copied string in names table and set it
        FastNameView::CharT* strCopy = new FastNameView::CharT[fview.size + 1];
        ::memcpy(strCopy, fview.str, fview.size);
        strCopy[fview.size] = '\0';

        view.str = strCopy;
        view.size = fview.size;

        // index will be a new row in names table
        size_t index = db->namesTable.size();
        db->namesTable.push_back(view);
        db->nameToIndexMap.emplace(view, index);

        db->sizeOfNames += (view.size * sizeof(FastNameView::CharT));
    }
}

template <>
bool AnyCompare<FastName>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<FastName>() == v2.Get<FastName>();
}

} // namespace DAVA
