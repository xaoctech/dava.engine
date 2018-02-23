#include "VisualScriptEditor/Private/Models/VisualScriptFlowScene.h"
#include "VisualScriptEditor/Private/Models/VisualScriptRegistryModel.h"
#include "VisualScriptEditor/Private/Models/VisualScriptNodeModel.h"

#include <nodes/DataModelRegistry>
#include <nodes/Node>
#include <nodes/NodeDataModel>

#include <VisualScript/VisualScript.h>
#include <VisualScript/VisualScriptPin.h>

namespace DAVA
{
VisualScriptFlowScene::VisualScriptFlowScene(ContextAccessor* accessor_, UI* ui_, VisualScript* script_, std::shared_ptr<QtNodes::DataModelRegistry> registry)
    : QtNodes::FlowScene(registry)
    , accessor(accessor_)
    , ui(ui_)
    , script(script_)
{
}

VisualScriptFlowScene::~VisualScriptFlowScene()
{
}

void VisualScriptFlowScene::SaveRuntime() const
{
    for (auto const& pair : _nodes)
    {
        const std::unique_ptr<QtNodes::Node>& node = pair.second;
        const VisualScriptNodeModel* nodeModel = static_cast<const VisualScriptNodeModel*>(node->nodeDataModel());
        const QtNodes::NodeGraphicsObject& graphicsObject = node->nodeGraphicsObject();
        QPointF position = graphicsObject.pos();

        VisualScriptNode* visualScriptNode = nodeModel->GetScriptNode();
        visualScriptNode->position.x = position.x();
        visualScriptNode->position.y = position.y();
    }
}

void VisualScriptFlowScene::LoadRuntime()
{
    const Vector<VisualScriptNode*>& existingNodes = script->GetNodes();
    size_t nodesCount = existingNodes.size();

    Vector<std::unique_ptr<QtNodes::Node>> createdNodes;
    createdNodes.reserve(nodesCount);

    for (VisualScriptNode* scriptNode : existingNodes)
    { // restore visual part of nodes
        std::unique_ptr<QtNodes::NodeDataModel> dataModel = std::make_unique<VisualScriptNodeModel>(accessor, ui, script, scriptNode);
        VisualScriptNodeModel* nodeDataModel = static_cast<VisualScriptNodeModel*>(dataModel.get());

        std::unique_ptr<QtNodes::Node> node = std::make_unique<QtNodes::Node>(std::move(dataModel));
        std::unique_ptr<QtNodes::NodeGraphicsObject> ngo = std::make_unique<QtNodes::NodeGraphicsObject>(*this, *node);

        VisualScriptNode* visualScriptNode = nodeDataModel->GetScriptNode();
        QPointF position(visualScriptNode->position.x, visualScriptNode->position.y);
        ngo->setPos(position);
        node->setGraphicsObject(std::move(ngo));

        auto nodePtr = node.get();
        createdNodes.push_back(std::move(node));

        emit nodeCreated(*nodePtr);
    }

    auto findVisualNode = [nodesCount, &existingNodes, &createdNodes](const VisualScriptNode* scriptNode)
    {
        for (size_t index = 0; index < nodesCount; ++index)
        {
            if (existingNodes[index] == scriptNode)
            {
                return createdNodes[index].get();
            }
        }
        return static_cast<QtNodes::Node*>(nullptr);
    };

    for (size_t index = 0; index < nodesCount; ++index)
    { // restore connections
        VisualScriptNode* outScriptNode = existingNodes[index];
        QtNodes::Node* nodeOut = createdNodes[index].get();

        const Vector<VisualScriptPin*>& outputPins = outScriptNode->GetAllOutputPins();
        size_t outPinCount = outputPins.size();
        for (size_t outPinIndex = 0; outPinIndex < outPinCount; ++outPinIndex)
        {
            QtNodes::PortIndex portIndexOut = static_cast<QtNodes::PortIndex>(outPinIndex);
            VisualScriptPin* outputPin = outputPins[outPinIndex];
            QtNodes::PortKind portKindOut = VisualScriptNodeModel::GetPortKind(outputPin);

            const Set<VisualScriptPin*>& connectedPins = outputPin->GetConnectedSet();
            for (VisualScriptPin* inputPin : connectedPins)
            {
                VisualScriptNode* connectedScriptNode = inputPin->GetSerializationOwner();
                QtNodes::Node* nodeIn = findVisualNode(connectedScriptNode);
                QtNodes::PortKind portKindIn = VisualScriptNodeModel::GetPortKind(inputPin);

                DVASSERT(nodeOut != nullptr);

                const Vector<VisualScriptPin*>& inputPins = connectedScriptNode->GetAllInputPins();
                for (size_t portIndexIn = 0; portIndexIn < inputPins.size(); ++portIndexIn)
                {
                    if (inputPins[portIndexIn] == inputPin)
                    {
                        std::shared_ptr<QtNodes::Connection> connection = std::make_shared<QtNodes::Connection>(*nodeIn,
                                                                                                                static_cast<QtNodes::PortIndex>(portIndexIn),
                                                                                                                portKindIn,
                                                                                                                *nodeOut,
                                                                                                                portIndexOut,
                                                                                                                portKindOut);

                        std::unique_ptr<QtNodes::ConnectionGraphicsObject> cgo = std::make_unique<QtNodes::ConnectionGraphicsObject>(*this, *connection);

                        nodeIn->nodeState().setConnection(QtNodes::PortType::In, static_cast<QtNodes::PortIndex>(portIndexIn), *connection);
                        nodeOut->nodeState().setConnection(QtNodes::PortType::Out, portIndexOut, *connection);

                        // after this function connection points are set to node port
                        connection->setGraphicsObject(std::move(cgo));
                        _connections[connection->id()] = connection;

                        emit connectionCreated(*connection);
                        break;
                    }
                }
            }
        }
    }

    { // store created nodes at internal data
        for (size_t index = 0; index < nodesCount; ++index)
        {
            QtNodes::Node* nodePtr = createdNodes[index].get();
            _nodes[nodePtr->id()] = std::move(createdNodes[index]);
        }
        createdNodes.clear();
    }
}

void VisualScriptFlowScene::NodeWillBeRemoved(QtNodes::NodeDataModel* model)
{
    VisualScriptNodeModel* nodeModel = static_cast<VisualScriptNodeModel*>(model);
    VisualScriptNode* visualScriptNode = nodeModel->GetScriptNode();

    script->RemoveNode(visualScriptNode);
}

} //DAVA
