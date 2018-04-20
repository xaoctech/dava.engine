#pragma once

#include <Entity/SingleComponent.h>

namespace DAVA
{
class TEMPLATESingleComponent : public SingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(TEMPLATESingleComponent, SingleComponent);

    TEMPLATESingleComponent();
    ~TEMPLATESingleComponent();

    void Clear() override;
};
}
