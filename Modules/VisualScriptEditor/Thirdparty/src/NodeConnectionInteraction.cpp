#include "NodeConnectionInteraction.hpp"

#include "ConnectionGraphicsObject.hpp"
#include "NodeGraphicsObject.hpp"
#include "NodeDataModel.hpp"
#include "DataModelRegistry.hpp"
#include "FlowScene.hpp"

using QtNodes::NodeConnectionInteraction;
using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::FlowScene;
using QtNodes::Node;
using QtNodes::Connection;
using QtNodes::NodeDataModel;

NodeConnectionInteraction::
NodeConnectionInteraction(Node& node, Connection& connection, FlowScene& scene)
    : _node(&node)
    , _connection(&connection)
    , _scene(&scene)
{
}

bool
NodeConnectionInteraction::
canConnect(PortIndex& portIndex, bool& typeConversionNeeded, std::unique_ptr<NodeDataModel>& converterModel) const
{
    typeConversionNeeded = false;

    // 1) Connection requires a port

    PortType requiredPort = connectionRequiredPort();

    if (requiredPort == PortType::None)
    {
        return false;
    }

    // 2) connection point is on top of the node port

    QPointF connectionPoint = connectionEndScenePosition(requiredPort);

    portIndex = nodePortIndexUnderScenePoint(requiredPort,
                                             connectionPoint);

    if (portIndex == INVALID)
    {
        return false;
    }

    // 3) Node port is vacant

    // port should be empty
    if (!nodePortIsEmpty(requiredPort, portIndex))
        return false;

    // 4) Connection type equals node port type, or there is a registered type conversion that can translate between the two

    auto const connectionDataType = _connection->dataType(oppositePort(requiredPort));

    auto const& modelTarget = _node->nodeDataModel();
    auto const candidateNodeDataType = modelTarget->dataType(requiredPort, portIndex);

    if (connectionDataType.id != candidateNodeDataType.id)
    {
        // First check types connection on model level

        NodeDataModel* inDataModel = modelTarget;
        NodeDataModel* outDataModel = _connection->getNode(oppositePort(requiredPort))->nodeDataModel();
        PortIndex inIndex = portIndex;
        PortIndex outIndex = _connection->getPortIndex(oppositePort(requiredPort));

        if (requiredPort == PortType::Out) // Need swap in/out data
        {
            std::swap(inDataModel, outDataModel);
            std::swap(inIndex, outIndex);
        }

        std::shared_ptr<NodeData> outData = outDataModel->outData(outIndex);
        bool const canConnect = inDataModel->canSetInData(outData, inIndex);

        // In second try find registered type conversion
        if (!canConnect)
        {
            if (requiredPort == PortType::In)
            {
                return typeConversionNeeded = (converterModel = _scene->registry().getTypeConverter(connectionDataType.id, candidateNodeDataType.id)) != nullptr;
            }
            return typeConversionNeeded = (converterModel = _scene->registry().getTypeConverter(candidateNodeDataType.id, connectionDataType.id)) != nullptr;
        }
    }

    return true;
}

bool
NodeConnectionInteraction::
tryConnect() const
{
    // 1) Check conditions from 'canConnect'
    PortIndex portIndex = INVALID;
    bool typeConversionNeeded = false;
    std::unique_ptr<NodeDataModel> typeConverterModel;

    if (!canConnect(portIndex, typeConversionNeeded, typeConverterModel))
    {
        return false;
    }

    /// 1.5) If the connection is possible but a type conversion is needed, add a converter node to the scene, and connect it properly
    if (typeConversionNeeded)
    {
        //Determining port types
        PortType requiredPort = connectionRequiredPort();
        PortType connectedPort = requiredPort == PortType::Out ? PortType::In : PortType::Out;

        //Get the node and port from where the connection starts
        auto outNode = _connection->getNode(connectedPort);
        auto outNodePortIndex = _connection->getPortIndex(connectedPort);

        PortKind outNodePortKind = _connection->portKind(connectedPort);
        PortKind sourceNodePortKind = _node->nodeDataModel()->portKind(requiredPort, portIndex);
        Q_ASSERT(outNodePortKind == sourceNodePortKind);

        //Creating the converter node
        Node& converterNode = _scene->createNode(std::move(typeConverterModel));

        //Calculate and set the converter node's position
        auto converterNodePos = NodeGeometry::calculateNodePositionBetweenNodePorts(portIndex, requiredPort, _node, outNodePortIndex, connectedPort, outNode, converterNode);
        converterNode.nodeGraphicsObject().setPos(converterNodePos);

        //Connecting the converter node to the two nodes trhat originally supposed to be connected.
        //The connection order is different based on if the users connection was started from an input port, or an output port.
        if (requiredPort == PortType::In)
        {
            _scene->createConnection(converterNode, 0, outNodePortKind, *outNode, outNodePortIndex, outNodePortKind);
            _scene->createConnection(*_node, portIndex, sourceNodePortKind, converterNode, 0, sourceNodePortKind);
        }
        else
        {
            _scene->createConnection(converterNode, 0, sourceNodePortKind, *_node, portIndex, sourceNodePortKind);
            _scene->createConnection(*outNode, outNodePortIndex, outNodePortKind, converterNode, 0, outNodePortKind);
        }

        //Delete the users connection, we already replaced it.
        _scene->deleteConnection(*_connection);

        return true;
    }

    // 2) Assign node to required port in Connection

    PortType requiredPort = connectionRequiredPort();
    _node->nodeState().setConnection(requiredPort,
                                     portIndex,
                                     *_connection);

    // 3) Assign Connection to empty port in NodeState
    // The port is not longer required after this function
    PortKind portKind = _node->nodeDataModel()->portKind(requiredPort, portIndex);
    _connection->setNodeToPort(*_node, requiredPort, portIndex, portKind);

    // 4) Adjust Connection geometry

    _node->nodeGraphicsObject().moveConnections();

    // 5) Poke model to intiate data transfer

    auto outNode = _connection->getNode(PortType::Out);
    if (outNode)
    {
        PortIndex outPortIndex = _connection->getPortIndex(PortType::Out);
        outNode->onDataUpdated(outPortIndex);
    }

    return true;
}

/// 1) Node and Connection should be already connected
/// 2) If so, clear Connection entry in the NodeState
/// 3) Set Connection end to 'requiring a port'
bool
NodeConnectionInteraction::
disconnect(PortType portToDisconnect) const
{
    PortIndex portIndex =
    _connection->getPortIndex(portToDisconnect);

    NodeState& state = _node->nodeState();

    // clear pointer to Connection in the NodeState
    state.getEntries(portToDisconnect)[portIndex].clear();

    // 4) Propagate invalid data to IN node
    _connection->propagateEmptyData();

    // clear Connection side
    _connection->clearNode(portToDisconnect);

    _connection->setRequiredPort(portToDisconnect);

    _connection->getConnectionGraphicsObject().grabMouse();

    return true;
}

// ------------------ util functions below

PortType
NodeConnectionInteraction::
connectionRequiredPort() const
{
    auto const& state = _connection->connectionState();

    return state.requiredPort();
}

QPointF
NodeConnectionInteraction::
connectionEndScenePosition(PortType portType) const
{
    auto& go =
    _connection->getConnectionGraphicsObject();

    ConnectionGeometry& geometry = _connection->connectionGeometry();

    QPointF endPoint = geometry.getEndPoint(portType);

    return go.mapToScene(endPoint);
}

QPointF
NodeConnectionInteraction::
nodePortScenePosition(PortType portType, PortIndex portIndex) const
{
    NodeGeometry const& geom = _node->nodeGeometry();

    QPointF p = geom.portScenePosition(portIndex, portType);

    NodeGraphicsObject& ngo = _node->nodeGraphicsObject();

    return ngo.sceneTransform().map(p);
}

PortIndex
NodeConnectionInteraction::
nodePortIndexUnderScenePoint(PortType portType,
                             QPointF const& scenePoint) const
{
    NodeGeometry const& nodeGeom = _node->nodeGeometry();

    QTransform sceneTransform =
    _node->nodeGraphicsObject().sceneTransform();

    PortIndex portIndex = nodeGeom.checkHitScenePoint(portType,
                                                      scenePoint,
                                                      sceneTransform);
    return portIndex;
}

bool
NodeConnectionInteraction::
nodePortIsEmpty(PortType portType, PortIndex portIndex) const
{
    NodeState const& nodeState = _node->nodeState();

    auto const& entries = nodeState.getEntries(portType);

    if (entries[portIndex].empty())
        return true;

    const auto connectionPolicy = _node->nodeDataModel()->connectionPolicy(portType, portIndex);
    //  return ( portType == PortType::Out && connectionPolicy == NodeDataModel::ConnectionPolicy::Many);
    return (connectionPolicy == NodeDataModel::ConnectionPolicy::Many);
}
