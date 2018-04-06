#include "VisualScriptEditor/Private/Models/VisualScriptNodeModel.h"
#include "VisualScriptEditor/Private/Models/NodePinData.h"
#include "VisualScriptEditor/Private/Models/TypeConversion.h"

#include <nodes/NodeData>

#include <TArc/Controls/Widget.h>
#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/DoubleSpinBox.h>
#include <TArc/Controls/ImageView.h>
#include <TArc/Controls/IntSpinBox.h>
#include <TArc/Controls/Label.h>
#include <TArc/Controls/LineEdit.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Controls/ReflectedWidget.h>
#include <TArc/Utils/Utils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <VisualScript/VisualScriptPin.h>

#include <Base/Result.h>
#include <Debug/DVAssert.h>
#include <Debug/Backtrace.h>
#include <Logger/Logger.h>
#include <Render/Image/Image.h>

#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>

#include <QMouseEvent>

namespace DAVA
{
struct VisualScriptNodeModel::PinWidgetDescriptor
{
    VisualScriptPin* pin = nullptr;
    QWidget* widget = nullptr;
    QWidget* additionalWidget = nullptr;
    bool isSquare = false;
    bool customPlaceholder = false;

    Any GetDefaultValue() const
    {
        Any retValue;
        if (pin)
        {
            retValue = pin->GetDefaultValue();
        }

        if (retValue.IsEmpty() && customPlaceholder == true)
        {
            retValue = GetPlaceHolderString();
        }

        return retValue;
    }

    void SetDefaultValue(Any defaultValue)
    {
        if (pin != nullptr)
        {
            pin->SetDefaultValue(defaultValue);
        }
    }

    void ResetDefaultValue()
    {
        SetDefaultValue(Any());
    }

    bool IsDefaultValueNotEmpty() const
    {
        if (pin != nullptr)
        {
            return pin->GetDefaultValue().IsEmpty() == false;
        }
        return false;
    }

    bool IsWidgetVisible() const
    {
        if (pin != nullptr)
        {
            return pin->GetConnectedSet().empty(); //return pin->GetConnectedTo() == nullptr;
        }

        return false;
    }

    String GetPlaceHolderString() const
    {
        return "Value not set";
    }

    DAVA_REFLECTION(VisualScriptNodeModel::PinWidgetDescriptor)
    {
        ReflectionRegistrator<VisualScriptNodeModel::PinWidgetDescriptor>::Begin()
        .Field("defaultValue", &VisualScriptNodeModel::PinWidgetDescriptor::GetDefaultValue, &VisualScriptNodeModel::PinWidgetDescriptor::SetDefaultValue)
        .Field("isWidgetVisible", &VisualScriptNodeModel::PinWidgetDescriptor::IsWidgetVisible, nullptr) //{ return pin->GetConnectedTo() == nullptr; }, nullptr)

        .Field("placeHolderString", &VisualScriptNodeModel::PinWidgetDescriptor::GetPlaceHolderString, nullptr)

        .Method("resetDefaultValue", &VisualScriptNodeModel::PinWidgetDescriptor::ResetDefaultValue)
        .Field("resetDefaultValueIcon", []() { return SharedIcon(":/VisualScriptEditor/Icons/delete.png"); }, nullptr)
        .Field("resetDefaultValueEnabled", &VisualScriptNodeModel::PinWidgetDescriptor::IsDefaultValueNotEmpty, nullptr)
        .End();
    }
};

VisualScriptNodeModel::VisualScriptNodeModel(ContextAccessor* accessor_, UI* ui_, VisualScript* script_, VisualScriptNode* visualScriptNode_)
    : QtNodes::NodeDataModel()
    , visualScriptNode(visualScriptNode_)
    , accessor(accessor_)
    , ui(ui_)
    , script(script_)
{
    DVASSERT(visualScriptNode);

    size_t portsTypesCount = PortToIndex(QtNodes::PortType::Count);
    portWidgetDescriptors.resize(portsTypesCount);

    Vector<bool> hasWidgets(portsTypesCount, false);
    for (size_t portType = 0; portType < portsTypesCount; ++portType)
    {
        QtNodes::PortType qnPortType = static_cast<QtNodes::PortType>(portType);
        size_t portsCount = nPorts(qnPortType);
        portWidgetDescriptors[portType].resize(portsCount);

        for (size_t portIndex = 0; portIndex < portsCount; ++portIndex)
        {
            QtNodes::PortIndex qnPortIndex = static_cast<QtNodes::PortIndex>(portIndex);
            VisualScriptPin* pin = GetPin(qnPortType, qnPortIndex);
            DVASSERT(pin != nullptr);

            Reflection inReflection = Reflection::Create(ReflectedObject(&portWidgetDescriptors[portType][portIndex]));
            QtHBoxLayout* layout = nullptr;

            const Type* pinType = pin->GetType();
            if ((qnPortType == QtNodes::PortType::In) && (pinType != nullptr && pin->HasDefaultValue()))
            {
                const Type* valueType = pinType->Decay();
                if (valueType == Type::Instance<bool>())
                {
                    CheckBox::Params params(accessor, ui, DAVA::mainWindowKey);
                    params.fields[CheckBox::Fields::Checked] = "defaultValue";
                    params.fields[CheckBox::Fields::EmptyValue].BindConstValue(Qt::PartiallyChecked);
                    params.fields[CheckBox::Fields::IsVisible] = "defaultValueVisible";
                    CheckBox* checkBox = new CheckBox(params, accessor, inReflection, nullptr);

                    layout = new QtHBoxLayout();
                    layout->AddControl(checkBox);

                    portWidgetDescriptors[portType][portIndex].isSquare = true;
                }
                else if (valueType == Type::Instance<String>()
                         || valueType == Type::Instance<WideString>()
                         || valueType == Type::Instance<FastName>())
                {
                    LineEdit::Params params(accessor, ui, DAVA::mainWindowKey);
                    params.fields[LineEdit::Fields::Text] = "defaultValue";
                    params.fields[LineEdit::Fields::PlaceHolder] = "placeHolderString";
                    LineEdit* lineEdit = new LineEdit(params, accessor, inReflection, nullptr);

                    layout = new QtHBoxLayout();
                    layout->AddControl(lineEdit);
                }
                else if (valueType == Type::Instance<int8>()
                         || valueType == Type::Instance<uint8>()
                         || valueType == Type::Instance<int16>()
                         || valueType == Type::Instance<uint16>()
                         || valueType == Type::Instance<int32>()
                         || valueType == Type::Instance<uint32>())
                {
                    IntSpinBox::Params params(accessor, ui, DAVA::mainWindowKey);
                    params.fields[IntSpinBox::Fields::Value] = "defaultValue";
                    IntSpinBox* spinBox = new IntSpinBox(params, accessor, inReflection, nullptr);

                    layout = new QtHBoxLayout();
                    layout->AddControl(spinBox);

                    portWidgetDescriptors[portType][portIndex].customPlaceholder = true;
                }
                else if (valueType == Type::Instance<float32>()
                         || valueType == Type::Instance<float64>())
                {
                    DoubleSpinBox::Params params(accessor, ui, DAVA::mainWindowKey);
                    params.fields[DoubleSpinBox::Fields::Value] = "defaultValue";
                    DoubleSpinBox* spinBox = new DoubleSpinBox(params, accessor, inReflection, nullptr);

                    layout = new QtHBoxLayout();
                    layout->AddControl(spinBox);

                    portWidgetDescriptors[portType][portIndex].customPlaceholder = true;
                }
                else
                {
                    Logger::Error("[%s] Cannot create control for type %s", __FUNCTION__, pinType->GetName()); //enable it later
                }
            }
            else if ((qnPortType == QtNodes::PortType::Out) && (pinType != nullptr))
            {
                const Type* valueType = pinType->Decay();
                if (valueType == Type::Instance<Image*>())
                {
                    Reflection outReflection = Reflection::Create(ReflectedObject(pin));

                    ImageView::Params params(accessor, ui, DAVA::mainWindowKey);
                    params.fields[ImageView::Fields::Image] = "pinValue";
                    ImageView* imageView = new ImageView(params, accessor, outReflection, nullptr);

                    portWidgetDescriptors[portType][portIndex].isSquare = true;
                    portWidgetDescriptors[portType][portIndex].widget = imageView->ToWidgetCast();
                    portWidgetDescriptors[portType][portIndex].pin = pin;
                }
            }

            if (layout != nullptr)
            {
                ReflectedButton::Params btnParams(accessor, ui, DAVA::mainWindowKey);
                btnParams.fields[ReflectedButton::Fields::Clicked] = "resetDefaultValue";
                btnParams.fields[ReflectedButton::Fields::Icon] = "resetDefaultValueIcon";
                btnParams.fields[ReflectedButton::Fields::Enabled] = "resetDefaultValueEnabled";
                ReflectedButton* resetButton = new ReflectedButton(btnParams, accessor, inReflection, nullptr);
                layout->AddControl(resetButton);
                portWidgetDescriptors[portType][portIndex].additionalWidget = resetButton->ToWidgetCast();

                ReflectedWidget::Params phParams(accessor, ui, DAVA::mainWindowKey);
                phParams.fields[ReflectedWidget::Fields::Visible] = "isWidgetVisible";

                ReflectedWidget* placeHolder = new ReflectedWidget(phParams, accessor, inReflection, nullptr);
                portWidgetDescriptors[portType][portIndex].widget = placeHolder->ToWidgetCast();
                portWidgetDescriptors[portType][portIndex].widget->setLayout(layout);

                portWidgetDescriptors[portType][portIndex].pin = pin;

                layout->setSpacing(0);
                layout->setMargin(0);
            }

            hasWidgets[portType] = hasWidgets[portType] || (portWidgetDescriptors[portType][portIndex].widget != nullptr);
        }
    }

    bool needToCreateHolder = std::any_of(hasWidgets.begin(), hasWidgets.end(), [](bool v) { return v; });
    if (needToCreateHolder)
    {
        class EventAcceptorWidget : public QWidget
        {
        public:
            EventAcceptorWidget()
                : QWidget(nullptr)
            {
            }

        protected:
            void mousePressEvent(QMouseEvent* e) override
            {
                QWidget::mousePressEvent(e);
                e->accept();
            }
        };

        containerWidget = new EventAcceptorWidget();
        containerWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        containerWidget->setAttribute(Qt::WA_NoSystemBackground);
        containerWidget->setAttribute(Qt::WA_TranslucentBackground);

        QHBoxLayout* boxLayout = new QHBoxLayout(containerWidget);
        boxLayout->setMargin(0);
        boxLayout->setSpacing(0);

        gridLayouts.resize(portsTypesCount, nullptr);
        for (size_t portType = 0; portType < portsTypesCount; ++portType)
        {
            if (hasWidgets[portType] == false)
            {
                continue;
            }

            gridLayouts[portType] = new QGridLayout();
            gridLayouts[portType]->setAlignment(static_cast<Qt::Alignment>(Qt::AlignLeft | Qt::AlignVCenter));

            gridLayouts[portType]->setMargin(0);
            gridLayouts[portType]->setSpacing(5);
            gridLayouts[portType]->setContentsMargins(0, 0, 0, 0);

            QtNodes::PortType qnPortType = static_cast<QtNodes::PortType>(portType);
            size_t portsCount = nPorts(qnPortType);
            for (size_t portIndex = 0; portIndex < portsCount; ++portIndex)
            {
                int32 index = static_cast<int32>(portIndex);
                if (portWidgetDescriptors[portType][portIndex].widget != nullptr)
                {
                    gridLayouts[portType]->addWidget(portWidgetDescriptors[portType][portIndex].widget, index, 0);
                }
                else
                {
                    QWidget* dummyWidget = new QWidget();
                    dummyWidget->setAttribute(Qt::WA_NoSystemBackground);
                    dummyWidget->setAttribute(Qt::WA_TranslucentBackground);

                    gridLayouts[portType]->addWidget(dummyWidget, index, 0);
                }
            }
            boxLayout->addLayout(gridLayouts[portType]);
        }
    }
}

VisualScriptNodeModel::~VisualScriptNodeModel()
{
}

QString VisualScriptNodeModel::name() const
{
    return category + ": " + QString(visualScriptNode->GetName().c_str());
}

QString VisualScriptNodeModel::caption() const
{
    return QString(visualScriptNode->GetName().c_str());
}

QColor VisualScriptNodeModel::captionColor() const
{
    static Vector<QColor> captionColors =
    {
      QColor(0, 0, 0), // NONE
      QColor(0, 150, 0), // GET_VAR
      QColor(0, 255, 0), // SET_VAR
      QColor(100, 0, 0), // FUNCTION
      QColor(0, 100, 100), // BRANCH
      QColor(150, 150, 0), // WHILE
      QColor(150, 150, 0), // DO_N
      QColor(150, 150, 0), // FOR
      QColor(150, 150, 150), // WAIT
      QColor(0, 0, 255), // EVENT
      QColor(0, 0, 200), // CUSTOM_EVENT
      QColor(50, 0, 200), // ANOTHER_SCRIPT
      QColor(200, 200, 200), // GET_MEMBER
      QColor(200, 200, 200), // SET_MEMBER
    };

    DVASSERT(captionColors.size() == VisualScriptNode::TYPE_COUNT);
    return captionColors[visualScriptNode->GetType()];
}

bool VisualScriptNodeModel::captionVisible() const
{
    return true;
}

QJsonObject VisualScriptNodeModel::save() const
{
    QJsonObject modelJson;
    modelJson["name"] = name();
    return modelJson;
}

QString VisualScriptNodeModel::portCaption(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    VisualScriptPin* pin = GetPin(portType, index);
    DVASSERT(pin != nullptr);

    QString caption;
    QtNodes::NodeDataType dataType = DAVATypeToNodeType(pin);
    if (dataType.prettyName.isEmpty() == false)
    {
        caption = dataType.prettyName + " ";
    }

    caption += QString::fromLatin1(pin->GetName().c_str());
    return caption;
}

bool VisualScriptNodeModel::portCaptionVisible(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    return true;
}

QList<QString> VisualScriptNodeModel::portCaptions(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    VisualScriptPin* pin = GetPin(portType, index);
    DVASSERT(pin != nullptr);

    QList<QString> captions;
    captions << QString::fromLatin1(pin->GetName().c_str());

    if (pin->GetType() != nullptr)
    {
        captions << QString::fromLatin1(pin->GetType()->GetName());
    }

    return captions;
}

QString VisualScriptNodeModel::portHint(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    VisualScriptPin* pin = GetPin(portType, index);
    DVASSERT(pin != nullptr);

    QString hint;
    if (pin->GetType() != nullptr)
    {
        const String demangledName = DAVA::Debug::DemangleFrameSymbol(pin->GetType()->GetName());
        hint = "type: " + QString::fromLatin1(demangledName.c_str());
    }

    return hint;
}

unsigned int VisualScriptNodeModel::nPorts(QtNodes::PortType portType) const
{
    switch (portType)
    {
    case QtNodes::PortType::In:
        return static_cast<unsigned int>(visualScriptNode->GetAllInputPins().size());

    case QtNodes::PortType::Out:
        return static_cast<unsigned int>(visualScriptNode->GetAllOutputPins().size());

    default:
        DVASSERT(false);
        break;
    }

    return 0;
}

QtNodes::PortKind VisualScriptNodeModel::portKind(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
{
    VisualScriptPin* pin = GetPin(portType, portIndex);
    return GetPortKind(pin);
}

QtNodes::PortKind VisualScriptNodeModel::GetPortKind(VisualScriptPin* pin)
{
    DVASSERT(pin != nullptr);
    if (pin->IsDataPin())
    {
        return QtNodes::PortKind::Data;
    }
    return QtNodes::PortKind::Execution;
}

QtNodes::NodeDataType VisualScriptNodeModel::dataType(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    VisualScriptPin* pin = GetPin(portType, index);
    return DAVATypeToNodeType(pin);
}

std::shared_ptr<QtNodes::NodeData> VisualScriptNodeModel::outData(QtNodes::PortIndex portIndex)
{
    VisualScriptPin* pin = GetPin(QtNodes::PortType::Out, portIndex);
    DVASSERT(pin != nullptr);

    std::shared_ptr<QtNodes::NodeData> ret = std::make_shared<NodePinData>(pin);
    return ret;
}

QtNodes::NodeDataModel::ConnectionPolicy VisualScriptNodeModel::connectionPolicy(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
{
    VisualScriptPin* pin = GetPin(portType, portIndex);
    DVASSERT(pin != nullptr);

    if (portType == QtNodes::PortType::In)
    {
        if (pin->IsDataPin())
        {
            return QtNodes::NodeDataModel::ConnectionPolicy::One;
        }
        else
        {
            return QtNodes::NodeDataModel::ConnectionPolicy::Many;
        }
    }
    else if (portType == QtNodes::PortType::Out)
    {
        if (pin->IsDataPin())
        {
            return QtNodes::NodeDataModel::ConnectionPolicy::Many;
        }
        else
        {
            return QtNodes::NodeDataModel::ConnectionPolicy::One;
        }
    }

    return QtNodes::NodeDataModel::connectionPolicy(portType, portIndex);
}

void VisualScriptNodeModel::disconnectInData(std::shared_ptr<QtNodes::NodeData> outNodeData, QtNodes::PortIndex portIndexIn)
{
    VisualScriptPin* pin = GetPin(QtNodes::PortType::In, portIndexIn);
    DVASSERT(pin != nullptr);

    std::shared_ptr<NodePinData> outPinData = std::dynamic_pointer_cast<NodePinData>(outNodeData);
    if (outPinData)
    {
        VisualScriptPin* outPin = outPinData->GetPin();
        DVASSERT(VisualScriptPin::IsConnected(pin, outPin));

        pin->Disconnect(outPin);
    }
    else
    {
        DVASSERT(false);
    }
}

void VisualScriptNodeModel::setInData(std::shared_ptr<QtNodes::NodeData> data, QtNodes::PortIndex portIndex)
{
    VisualScriptPin* pin = GetPin(QtNodes::PortType::In, portIndex);
    DVASSERT(pin != nullptr);

    std::shared_ptr<NodePinData> newData = std::dynamic_pointer_cast<NodePinData>(data);
    if (newData)
    {
        VisualScriptPin* outPin = newData->GetPin();

        const Set<VisualScriptPin*>& connections = pin->GetConnectedSet();
        if (connections.count(outPin) == 0)
        {
            bool connectSuccessful = pin->Connect(outPin);
            if (connectSuccessful == false)
            {
                Logger::Error("Cannot connect pins");
            }
        }
    }
    else
    {
        if (pin->IsDataPin())
        {
            DVASSERT(pin->GetConnectedTo() == nullptr);
        }
    }
}

bool VisualScriptNodeModel::canSetInData(std::shared_ptr<QtNodes::NodeData> dataNode, QtNodes::PortIndex portIndex) const
{
    VisualScriptPin* inPin = GetPin(QtNodes::PortType::In, portIndex);
    DVASSERT(inPin != nullptr);

    std::shared_ptr<NodePinData> newData = std::dynamic_pointer_cast<NodePinData>(dataNode);
    if (newData)
    {
        VisualScriptPin* outPin = newData->GetPin();
        return VisualScriptPin::CanConnect(inPin, outPin) != VisualScriptPin::CanConnectResult::CANNOT_CONNECT;
    }

    return false;
}

QWidget* VisualScriptNodeModel::embeddedWidget()
{
    return containerWidget;
}

VisualScriptPin* VisualScriptNodeModel::GetPin(QtNodes::PortType portType, QtNodes::PortIndex index) const
{
    DVASSERT(index < static_cast<QtNodes::PortIndex>(nPorts(portType)));

    VisualScriptPin* pin = nullptr;
    switch (portType)
    {
    case QtNodes::PortType::In:
        pin = visualScriptNode->GetAllInputPins()[static_cast<size_t>(index)];
        break;

    case QtNodes::PortType::Out:
        pin = visualScriptNode->GetAllOutputPins()[static_cast<size_t>(index)];
        break;

    default:
        DVASSERT(false);
        break;
    }

    return pin;
}

VisualScriptNode* VisualScriptNodeModel::GetScriptNode() const
{
    return visualScriptNode;
}

std::unique_ptr<QtNodes::NodeDataModel> VisualScriptNodeModel::clone() const
{
    DAVA_THROW(Exception, "Clone should not be called.");
    //    return std::unique_ptr<QtNodes::NodeDataModel>();
}

void VisualScriptNodeModel::setCategory(const QString& category_)
{
    category = category_;
}

void VisualScriptNodeModel::setEntryHeight(unsigned int height)
{
    if (containerWidget != nullptr)
    {
        size_t layoutsCount = gridLayouts.size();
        for (size_t layIndex = 0; layIndex < layoutsCount; ++layIndex)
        {
            if (gridLayouts[layIndex] != nullptr)
            {
                size_t rowsCount = portWidgetDescriptors[layIndex].size();
                for (size_t row = 0; row < rowsCount; ++row)
                {
                    VisualScriptNodeModel::PinWidgetDescriptor& descr = portWidgetDescriptors[layIndex][row];
                    if (descr.isSquare == true)
                    {
                        descr.widget->setFixedHeight(height);

                        int width = height;
                        if (descr.additionalWidget != nullptr)
                        {
                            width += height;
                        }

                        descr.widget->setFixedWidth(width);
                    }

                    gridLayouts[layIndex]->setRowMinimumHeight(static_cast<int32>(row), height);
                }
            }
        }
    }
}

void VisualScriptNodeModel::setSpacing(unsigned int spacing)
{
    if (containerWidget != nullptr)
    {
        int value = static_cast<int>(spacing);

        for (QGridLayout* layout : gridLayouts)
        {
            if (layout != nullptr)
            {
                layout->setVerticalSpacing(value);
            }
        }
    }
}

QtNodes::NodeValidationState VisualScriptNodeModel::validationState() const
{
    if (visualScriptNode != nullptr)
    {
        Result res = visualScriptNode->GetCompileResult();
        switch (res.type)
        {
        case Result::RESULT_SUCCESS:
            return QtNodes::NodeValidationState::Valid;
        case Result::RESULT_WARNING:
            return QtNodes::NodeValidationState::Warning;
        case Result::RESULT_ERROR:
            return QtNodes::NodeValidationState::Error;
        };
    }

    return QtNodes::NodeValidationState::Error;
}

QString VisualScriptNodeModel::validationMessage() const
{
    if (visualScriptNode != nullptr)
    {
        Result res = visualScriptNode->GetCompileResult();
        return QString::fromStdString(res.message);
    }

    return "visualScriptNode is nullptr";
}

} //DAVA
