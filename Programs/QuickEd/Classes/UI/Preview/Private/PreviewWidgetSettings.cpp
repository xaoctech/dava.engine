#include "UI/Preview/PreviewWidgetSettings.h"

#include <TArc/DataProcessing/PropertiesHolder.h>

#include <Math/Color.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PreviewWidgetSettings)
{
    DAVA::ReflectionRegistrator<PreviewWidgetSettings>::Begin()[DAVA::M::DisplayName("Preview widget"), DAVA::M::SettingsSortKey(70)]
    .ConstructorByPointer()
    .Field("backgroundColor0", &PreviewWidgetSettings::backgroundColor0)[DAVA::M::HiddenField()] // old field, left for converting old settings into new
    .Field("backgroundColor1", &PreviewWidgetSettings::backgroundColor1)[DAVA::M::HiddenField()] // old field, left for converting old settings into new
    .Field("backgroundColor2", &PreviewWidgetSettings::backgroundColor2)[DAVA::M::HiddenField()] // old field, left for converting old settings into new
    .Field("backgroundColors", &PreviewWidgetSettings::backgroundColors)[DAVA::M::DisplayName("Background colors")]
    .Field("backgroundColorIndex", &PreviewWidgetSettings::backgroundColorIndex)[DAVA::M::DisplayName("Background color index"), DAVA::M::HiddenField()]
    .End();
}

namespace PreviewWidgetSettingsDetail
{
using namespace DAVA;
using namespace DAVA::TArc;

const Color defaultBackgroundColor0 = Color(0.0f, 0.0f, 0.0f, 0.5f);
const Color defaultBackgroundColor1 = Color(0.242f, 0.242f, 0.242f, 1.0f);
const Color defaultBackgroundColor2 = Color(0.159f, 0.159f, 0.159f, 1.0f);

void LoadIntoReflection(Reflection::Field& field, const PropertiesItem& node)
{
    Vector<Reflection::Field> fields = field.ref.GetFields();
    if (fields.empty())
    {
        Any value = node.Get(field.key.Cast<String>(), field.ref.GetValue(), field.ref.GetValueType());
        if (value.IsEmpty() == false)
        {
            field.ref.SetValueWithCast(value);
        }
    }
    else
    {
        for (Reflection::Field& f : fields)
        {
            PropertiesItem fieldItem = node.CreateSubHolder(f.key.Cast<String>());
            LoadIntoReflection(f, fieldItem);
        }
    }
}
} // namespace PreviewWidgetSettingsDetail

PreviewWidgetSettings::PreviewWidgetSettings()
    : backgroundColor0(PreviewWidgetSettingsDetail::defaultBackgroundColor0)
    , backgroundColor1(PreviewWidgetSettingsDetail::defaultBackgroundColor1)
    , backgroundColor2(PreviewWidgetSettingsDetail::defaultBackgroundColor2)
{
}

void PreviewWidgetSettings::Load(const DAVA::TArc::PropertiesItem& settingsItem)
{
    using namespace DAVA;
    using namespace DAVA::TArc;
    using namespace PreviewWidgetSettingsDetail;

    Reflection::Field rootField;
    rootField.ref = Reflection::Create(ReflectedObject(this));
    rootField.key = String("Root");

    {
        String colorsFieldName = "backgroundColors";
        Reflection::Field colorsField;
        colorsField.key = colorsFieldName;
        colorsField.ref = rootField.ref.GetField(colorsFieldName);
        PropertiesItem colorsItem = settingsItem.CreateSubHolder(colorsFieldName);

        Any value = colorsItem.Get(colorsFieldName, colorsField.ref.GetValue(), colorsField.ref.GetValueType());
        if (value.IsEmpty() || value.CanCast<Vector<Color>>() == false || value.Cast<Vector<Color>>().empty() == true)
        {
            Vector<Color> oldColorSettings = { defaultBackgroundColor0, defaultBackgroundColor1, defaultBackgroundColor2 };
            for (uint32 i : { 0, 1, 2 })
            {
                String colorFieldName = Format("backgroundColor%u", i);
                Reflection::Field colorField;
                colorField.key = colorFieldName;
                colorField.ref = rootField.ref.GetField(colorFieldName);
                PropertiesItem colorItem = settingsItem.CreateSubHolder(colorFieldName);

                LoadIntoReflection(colorField, colorItem);
                oldColorSettings[i] = colorField.ref.GetValue().Cast<Color>();
            }

            value = oldColorSettings;
        }

        colorsField.ref.SetValueWithCast(value);
    }

    {
        String indexFieldName = "backgroundColorIndex";
        Reflection::Field indexField;
        indexField.key = indexFieldName;
        indexField.ref = rootField.ref.GetField(indexFieldName);
        PropertiesItem indexItem = settingsItem.CreateSubHolder(indexFieldName);

        Any value = indexItem.Get(indexFieldName, indexField.ref.GetValue(), indexField.ref.GetValueType());
        if (value.IsEmpty() == false)
        {
            indexField.ref.SetValueWithCast(value);
        }
    }
}

void PreviewWidgetSettings::Save(DAVA::TArc::PropertiesItem& settingsItem) const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Reflection::Field rootField;
    rootField.ref = Reflection::Create(ReflectedObject(this));
    rootField.key = String("Root");

    {
        String colorsFieldName = "backgroundColors";
        Reflection::Field colorsField;
        colorsField.key = colorsFieldName;
        colorsField.ref = rootField.ref.GetField(colorsFieldName);
        PropertiesItem colorsItem = settingsItem.CreateSubHolder(colorsFieldName);

        colorsItem.Set(colorsFieldName, colorsField.ref.GetValue());
    }

    {
        String indexFieldName = "backgroundColorIndex";
        Reflection::Field indexField;
        indexField.key = indexFieldName;
        indexField.ref = rootField.ref.GetField(indexFieldName);
        PropertiesItem indexItem = settingsItem.CreateSubHolder(indexFieldName);

        indexItem.Set(indexFieldName, indexField.ref.GetValue());
    }
}
