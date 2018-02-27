#include "Classes/SplineEditor/Private/SplineEditorPPExtensions.h"

#include <REPlatform/Commands/SplineEditorCommands.h>

#include <TArc/Controls/PropertyPanel/PropertyPanelMeta.h>
#include <TArc/Utils/Utils.h>

namespace SplineEditorExtensionsDetail
{
using namespace DAVA;

class RemoveSplinePointProducer : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::PropertyNode>& node) const override;
    Info GetInfo() const override;
    bool OnlyForSingleSelection() const override;
    std::unique_ptr<Command> CreateCommand(const std::shared_ptr<DAVA::PropertyNode>& node, const Params& params) const override;
};

bool RemoveSplinePointProducer::IsApplyable(const std::shared_ptr<DAVA::PropertyNode>& node) const
{
    return true;
}

M::CommandProducer::Info RemoveSplinePointProducer::GetInfo() const
{
    Info info;
    info.icon = SharedIcon(":/QtIcons/remove.png");
    info.tooltip = "Remove Ignore Value";
    return info;
}

bool RemoveSplinePointProducer::OnlyForSingleSelection() const
{
    return true;
}

std::unique_ptr<Command> RemoveSplinePointProducer::CreateCommand(const std::shared_ptr<DAVA::PropertyNode>& node, const Params& params) const
{
    size_t indexOfRemovedValue = static_cast<size_t>(node->sortKey);

    const std::shared_ptr<DAVA::PropertyNode> pointsNode = node->parent.lock();
    DVASSERT(pointsNode.get() != nullptr);

    const std::shared_ptr<DAVA::PropertyNode> splineComponentNode = pointsNode->parent.lock();
    DVASSERT(splineComponentNode.get() != nullptr);
    SplineComponent* spline = splineComponentNode->field.ref.GetValueObject().GetPtr<SplineComponent>();

    const std::shared_ptr<DAVA::PropertyNode> entityNode = splineComponentNode->parent.lock();
    DVASSERT(entityNode.get() != nullptr);
    Entity* entity = entityNode->field.ref.GetValueObject().GetPtr<Entity>();

    return std::make_unique<RemoveSplinePointCommand>(entity, spline, spline->controlPoints[indexOfRemovedValue], indexOfRemovedValue);
}
}

DAVA::M::CommandProducerHolder RemoveSplinePointCommandProducer()
{
    using namespace SplineEditorExtensionsDetail;
    DAVA::M::CommandProducerHolder holder;
    holder.AddCommandProducer(std::make_unique<RemoveSplinePointProducer>());
    return holder;
}
