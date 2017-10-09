#pragma once

#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"

#include <QTreeView>

class SceneTreeView : public QTreeView
{
public:
    struct Params
    {
        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::TArc::FieldDescriptor modelField;
    };

    SceneTreeView(const Params& params);

private:
    Params params;
    std::unique_ptr<DAVA::TArc::FieldBinder> binder;
};
