#pragma once

#include "Base/FastName.h"
#include "Base/UnordererSet.h"

namespace DAVA
{
/**
    Simple class for handling list of FastNames.

    Example:

        FastTags ft("tag1", "tag2", FastName("tag3"));
        ft.tags == UnorderedSet<FastName>{FastName("tag1"), FastName("tag2"), FastName("tag3")};
 */
class FastTags
{
    FastTags() = delete; // no empty tags by design.

public:
    template <typename T, typename... Args, typename = std::enable_if_t<!std::is_same<std::decay_t<T>, FastTags>::value>>
    FastTags(T&& t, Args&&... args)
        : tags({ FastName(std::forward<T>(t)), FastName(std::forward<Args>(args))... })
    {
    }

    FastTags(UnorderedSet<FastName> tags)
        : tags(std::move(tags))
    {
        DVASSERT(!this->tags.empty());
    }

    operator UnorderedSet<FastName>() const
    {
        return tags;
    }

    bool operator==(const FastTags& other) const
    {
        return tags == other.tags;
    }

    bool operator!=(const FastTags& other) const
    {
        return !(*this == other);
    }

    UnorderedSet<FastName> tags;
};
} // namespace DAVA