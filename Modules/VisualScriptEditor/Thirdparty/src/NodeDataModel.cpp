#include "NodeDataModel.hpp"

#include "StyleCollection.hpp"

using QtNodes::NodeDataModel;
using QtNodes::NodeStyle;
using QtNodes::PortType;
using QtNodes::PortIndex;

NodeDataModel::
NodeDataModel()
    : _nodeStyle(StyleCollection::nodeStyle())
{
    // Derived classes can initialize specific style here
}

QJsonObject
NodeDataModel::
save() const
{
    QJsonObject modelJson;

    modelJson["name"] = name();

    return modelJson;
}

NodeStyle const&
NodeDataModel::
nodeStyle() const
{
    return _nodeStyle;
}

void
NodeDataModel::
setNodeStyle(NodeStyle const& style)
{
    _nodeStyle = style;
}

NodeDataModel::ConnectionPolicy
NodeDataModel::connectionPolicy(PortType portType, PortIndex portIndex) const
{
    if (portType == PortType::Out)
        return NodeDataModel::ConnectionPolicy::Many;
    return NodeDataModel::ConnectionPolicy::One;
}
