#pragma once

namespace DAVA
{
struct UISingleComponent
{
    virtual ~UISingleComponent() = default;
    virtual void Clear() = 0;
};
}