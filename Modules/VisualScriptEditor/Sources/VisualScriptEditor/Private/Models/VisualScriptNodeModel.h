#pragma once

#include <nodes/NodeDataModel>

#include <TArc/Qt/QtString.h>

#include <Base/Array.h>
#include <Base/Vector.h>
#include <VisualScript/VisualScriptNode.h>

#include <QJsonObject>
#include <QList>
#include <QObject>

class QWidget;
class QGridLayout;
namespace DAVA
{
class ContextAccessor;
class UI;
class VisualScriptPin;
class VisualScript;
class VisualScriptNodeModel : public QtNodes::NodeDataModel
{
    Q_OBJECT

public:
    VisualScriptNodeModel(ContextAccessor* accessor, UI* ui, VisualScript* script, VisualScriptNode* visualScriptNode);
    ~VisualScriptNodeModel() override;

    QString name() const override;
    QString caption() const override;
    QColor captionColor() const override;

    bool captionVisible() const override;
    QJsonObject save() const override;
    void setCategory(const QString& category) override;

    std::unique_ptr<QtNodes::NodeDataModel> clone() const override;

    unsigned int nPorts(QtNodes::PortType portType) const override;
    QtNodes::PortKind portKind(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    static QtNodes::PortKind GetPortKind(VisualScriptPin* pin);

    QtNodes::NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex port) override;
    void setInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;
    bool canSetInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) const override;

    QtNodes::NodeDataModel::ConnectionPolicy connectionPolicy(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

    QString portCaption(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    bool portCaptionVisible(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    QList<QString> portCaptions(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    QString portHint(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;
    QWidget* embeddedWidget() override;

    void disconnectInData(std::shared_ptr<QtNodes::NodeData> nodeData, QtNodes::PortIndex portIndex) override;

    VisualScriptNode* GetScriptNode() const;

    void setEntryHeight(unsigned int height) override;
    void setSpacing(unsigned int spacing) override;

    QtNodes::NodeValidationState validationState() const override;
    QString validationMessage() const override;

private:
    VisualScriptNode* visualScriptNode = nullptr;
    VisualScriptPin* GetPin(QtNodes::PortType portType, QtNodes::PortIndex index) const;

    QString category;

    QWidget* containerWidget = nullptr;

    struct PinWidgetDescriptor;
    Vector<Vector<PinWidgetDescriptor>> portWidgetDescriptors;
    Vector<QGridLayout*> gridLayouts;

    ContextAccessor* accessor = nullptr;
    UI* ui = nullptr;
    VisualScript* script = nullptr;
};

} // DAVA
