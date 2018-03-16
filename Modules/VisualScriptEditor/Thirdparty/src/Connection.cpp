#include "Connection.hpp"

#include <iostream>
#include <math.h>

#include <QtWidgets/QtWidgets>
#include <QtGlobal>

#include "Node.hpp"
#include "FlowScene.hpp"
#include "FlowView.hpp"

#include "NodeGeometry.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"

#include "ConnectionState.hpp"
#include "ConnectionGeometry.hpp"
#include "ConnectionGraphicsObject.hpp"

using QtNodes::Connection;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::PortKind;
using QtNodes::ConnectionState;
using QtNodes::Node;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::ConnectionGraphicsObject;
using QtNodes::ConnectionGeometry;

Connection::
Connection(PortType portType,
           Node& node,
           PortIndex portIndex,
           PortKind portKind)
    : _id(QUuid::createUuid())
    , _connectionState()
{
    setNodeToPort(node, portType, portIndex, portKind);

    setRequiredPort(oppositePort(portType));

    conState = eConState::IS_CONNECTING;
}

Connection::
Connection(Node& nodeIn,
           PortIndex portIndexIn,
           PortKind portKindIn,
           Node& nodeOut,
           PortIndex portIndexOut,
           PortKind portKindOut)
    : _id(QUuid::createUuid())
    , _connectionState()
{
    setNodeToPort(nodeIn, PortType::In, portIndexIn, portKindIn);
    setNodeToPort(nodeOut, PortType::Out, portIndexOut, portKindOut);

    conState = eConState::IS_CONNECTED;
}

Connection::
~Connection()
{
    conState = eConState::NONE;

    propagateEmptyData();

    for (PortDescription& port : ports)
    {
        if (port.node != nullptr)
        {
            port.node->nodeGraphicsObject().update();
        }
    }
}

QJsonObject
Connection::
save() const
{
    QJsonObject connectionJson;

    for (size_t index = 0; index < PortTypeCount; ++index)
    {
        if (ports[index].node != nullptr)
        {
            QString preffix = PortPreffix(IndexToPort(index));

            connectionJson[preffix + "id"] = ports[index].node->id().toString();
            connectionJson[preffix + "index"] = ports[index].index;
            connectionJson[preffix + "kind"] = static_cast<int>(ports[index].kind);
        }
    }

    return connectionJson;
}

QUuid
Connection::
id() const
{
    return _id;
}

void
Connection::
setRequiredPort(PortType dragging)
{
    _connectionState.setRequiredPort(dragging);

    Q_ASSERT(dragging != PortType::None && dragging != PortType::Count);
    ports[PortToIndex(dragging)].node = nullptr;
    ports[PortToIndex(dragging)].index = INVALID;
}

PortType
Connection::
requiredPort() const
{
    return _connectionState.requiredPort();
}

void
Connection::
setGraphicsObject(std::unique_ptr<ConnectionGraphicsObject>&& graphics)
{
    _connectionGraphicsObject = std::move(graphics);

    // This function is only called when the ConnectionGraphicsObject
    // is newly created. At this moment both end coordinates are (0, 0)
    // in Connection G.O. coordinates. The position of the whole
    // Connection G. O. in scene coordinate system is also (0, 0).
    // By moving the whole object to the Node Port position
    // we position both connection ends correctly.

    if (requiredPort() != PortType::None)
    {
        PortType attachedPort = oppositePort(requiredPort());

        PortIndex attachedPortIndex = getPortIndex(attachedPort);

        auto node = getNode(attachedPort);

        QTransform nodeSceneTransform =
        node->nodeGraphicsObject().sceneTransform();

        QPointF pos = node->nodeGeometry().portScenePosition(attachedPortIndex,
                                                             attachedPort,
                                                             nodeSceneTransform);

        _connectionGraphicsObject->setPos(pos);
    }

    _connectionGraphicsObject->move();
}

PortIndex
Connection::
getPortIndex(PortType portType) const
{
    Q_ASSERT(portType != PortType::None && portType != PortType::Count);
    return ports[PortToIndex(portType)].index;
}

void
Connection::
setNodeToPort(Node& node,
              PortType portType,
              PortIndex portIndex,
              PortKind portKind)
{
    Q_ASSERT(portType != PortType::None && portType != PortType::Count);
    ports[PortToIndex(portType)].node = &node;
    ports[PortToIndex(portType)].index = portIndex;
    ports[PortToIndex(portType)].kind = portKind;

    _connectionState.setNoRequiredPort();

    updated(*this);
}

void
Connection::
removeFromNodes() const
{
    for (size_t index = 0; index < PortTypeCount; ++index)
    {
        if (ports[index].node != nullptr)
        {
            ports[index].node->nodeState().eraseConnection(IndexToPort(index), ports[index].index, id());
        }
    }
}

ConnectionGraphicsObject&
Connection::
getConnectionGraphicsObject() const
{
    return *_connectionGraphicsObject;
}

ConnectionState&
Connection::
connectionState()
{
    return _connectionState;
}

ConnectionState const&
Connection::
connectionState() const
{
    return _connectionState;
}

ConnectionGeometry&
Connection::
connectionGeometry()
{
    return _connectionGeometry;
}

ConnectionGeometry const&
Connection::
connectionGeometry() const
{
    return _connectionGeometry;
}

Node*
Connection::
getNode(PortType portType) const
{
    Q_ASSERT(portType != PortType::None && portType != PortType::Count);
    return ports[PortToIndex(portType)].node;
}

Node*&
Connection::
getNode(PortType portType)
{
    Q_ASSERT(portType != PortType::None && portType != PortType::Count);
    return ports[PortToIndex(portType)].node;
}

void
Connection::
clearNode(PortType portType)
{
    Q_ASSERT(portType != PortType::None && portType != PortType::Count);
    ports[PortToIndex(portType)].node = nullptr;
    ports[PortToIndex(portType)].index = INVALID;
    ports[PortToIndex(portType)].kind = PortKind::None;

    conState = eConState::IS_DISCONNECTING;
}

NodeDataType
Connection::
dataType(PortType portType) const
{
    Q_ASSERT(portType > PortType::None && portType < PortType::Count);
    const PortDescription& pDesc = ports[PortToIndex(portType)];
    if (pDesc.node != nullptr)
    {
        auto const& model = pDesc.node->nodeDataModel();
        return model->dataType(portType, pDesc.index);
    }
    return NodeDataType();
    Q_UNREACHABLE();
}

QtNodes::PortKind
Connection::
portKind(PortType portType) const
{
    Q_ASSERT(portType != PortType::None && portType != PortType::Count);
    return ports[PortToIndex(portType)].kind;
}

void
Connection::
propagateData(std::shared_ptr<NodeData> nodeData) const
{
    size_t portType = PortToIndex(PortType::In);
    if (ports[portType].node != nullptr)
    {
        ports[portType].node->propagateData(nodeData, ports[portType].index);
    }
}

void
Connection::
propagateEmptyData() const
{
    auto getModel = [this](size_t portType)
    {
        if (ports[portType].node != nullptr)
        {
            return ports[portType].node->nodeDataModel();
        }

        return static_cast<QtNodes::NodeDataModel*>(nullptr);
    };

    QtNodes::NodeDataModel* inModel = getModel(PortToIndex(PortType::In));
    QtNodes::NodeDataModel* outModel = getModel(PortToIndex(PortType::Out));
    if (inModel != nullptr && outModel != nullptr)
    {
        inModel->disconnectInData(outModel->outData(ports[PortToIndex(PortType::Out)].index), ports[PortToIndex(PortType::In)].index);
    }

    std::shared_ptr<NodeData> emptyData;
    propagateData(emptyData);
}
