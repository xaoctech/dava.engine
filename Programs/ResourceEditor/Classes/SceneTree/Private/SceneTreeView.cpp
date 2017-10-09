#include "Classes/SceneTree/Private/SceneTreeView.h"
#include "Classes/SceneTree/Private/SceneTreeModelV2.h"

SceneTreeView::SceneTreeView(const Params& params_)
    : QTreeView(nullptr)
    , params(params_)
{
    binder.reset(new DAVA::TArc::FieldBinder(params.accessor));
    binder->BindField(params.modelField, [this](const DAVA::Any& value) {
        SceneTreeModelV2* model = value.Get<SceneTreeModelV2*>(nullptr);

        QModelIndex rootIndex;
        if (model != nullptr)
        {
            rootIndex = model->GetRootIndex();
        }

        setModel(model);
        setRootIndex(rootIndex);
    });
}
