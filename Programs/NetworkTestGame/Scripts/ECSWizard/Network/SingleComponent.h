#pragma once

#include <Entity/SingletonComponent.h>

namespace DAVA
{
class TEMPLATESingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(TEMPLATESingleComponent, SingletonComponent);

    TEMPLATESingleComponent();
    ~TEMPLATESingleComponent();

    void Clear() override;
};
}
