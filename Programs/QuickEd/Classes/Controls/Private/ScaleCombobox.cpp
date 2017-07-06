#include "Controls/ScaleCombobox.h"

#include <TArc/Utils/ScopedValueGuard.h>
#include <TArc/DataProcessing/AnyQMetaType.h>

#include <Base/FastName.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectedMeta.h>

#include <QLineEdit>
#include <QSignalBlocker>
#include <QVariant>

namespace ScaleComboBoxDetails
{
const QString postfix(" %");
}

ScaleComboBox::ScaleComboBox(const Params& params, DAVA::TArc::ContextAccessor* accessor, DAVA::Reflection model, QWidget* parent)
    : ControlProxyImpl<QComboBox>(params, DAVA::TArc::ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ScaleComboBox::SetupControl()
{
    setEditable(true);

    connections.AddConnection(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), DAVA::MakeFunction(this, &ScaleComboBox::CurrentIndexChanged));
    connections.AddConnection(this->lineEdit(), &QLineEdit::editingFinished, DAVA::MakeFunction(this, &ScaleComboBox::EditingFinished));
    setSizeAdjustPolicy(QComboBox::AdjustToContents);

    QRegExp regEx("[0-8]?([0-9]|[0-9]){0,2}\\s?\\%?");
    this->setValidator(new QRegExpValidator(regEx));
    this->setInsertPolicy(QComboBox::NoInsert);
}

void ScaleComboBox::UpdateControl(const DAVA::TArc::ControlDescriptor& changedFields)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DVASSERT(updateControlProceed == false);
    ScopedValueGuard<bool> guard(updateControlProceed, true);

    if (changedFields.IsChanged(Fields::Enabled))
    {
        setEnabled(this->template GetFieldValue<bool>(Fields::Enabled, false));
    }

    if (changedFields.IsChanged(Fields::Enumerator))
    {
        CreateItems(model.GetField(changedFields.GetName(Fields::Enumerator)));
    }

    Any value = model.GetField(changedFields.GetName(Fields::Value)).GetValue();
    SetCurrentValue(value);
}

void ScaleComboBox::CreateItems(const DAVA::Reflection& fieldEnumerator)
{
    using namespace DAVA;

    QSignalBlocker blockSignals(this);

    clear();
    Vector<Reflection::Field> fields = fieldEnumerator.GetFields();
    for (Reflection::Field& field : fields)
    {
        Any fieldDescr = field.ref.GetValue();

        QVariant dataValue;
        dataValue.setValue(field.key);
        DVASSERT(fieldDescr.CanCast<float32>());
        float32 value = fieldDescr.Cast<float32>();

        addItem(ValueToString(value), dataValue);
    }
}

void ScaleComboBox::SetCurrentValue(const DAVA::Any& value)
{
    using namespace DAVA;

    QSignalBlocker blocker(this);
    if (value.CanGet<float32>())
    {
        float32 fltValue = value.Get<float32>();
        int index = this->findData(QVariant(fltValue));
        if (index != -1)
        {
            this->setCurrentIndex(index);
        }

        this->lineEdit()->setText(ValueToString(fltValue));
    }
    else
    {
        this->lineEdit()->setText(QString());
    }
}

void ScaleComboBox::CurrentIndexChanged(int newCurrentItem)
{
    if (updateControlProceed)
    {
        // ignore reaction on control initialization
        return;
    }

    wrapper.SetFieldValue(GetFieldName(Fields::Value), StringToValue(this->currentText()));
}

void ScaleComboBox::EditingFinished()
{
    if (updateControlProceed)
    {
        // ignore reaction on control initialization
        return;
    }

    wrapper.SetFieldValue(GetFieldName(Fields::Value), StringToValue(this->currentText()));
}

QString ScaleComboBox::ValueToString(DAVA::float32 value) const
{
    return QString("%1%2").arg(static_cast<int>(value * 100.0f + 0.5f)).arg(ScaleComboBoxDetails::postfix);
}

DAVA::float32 ScaleComboBox::StringToValue(const QString& text) const
{
    QString curTextValue = this->currentText();
    curTextValue.remove(ScaleComboBoxDetails::postfix);

    bool ok;
    float value = curTextValue.toFloat(&ok);
    DVASSERT(ok, "can not parse text to float");
    return value / 100.0f;
}
